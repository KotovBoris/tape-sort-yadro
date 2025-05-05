#include "file_tape.hpp"

#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <thread>

const std::size_t FileTape::CELL_SIZE = sizeof(int32_t);
std::size_t FileTape::tmp_counter_ = 0;

FileTape::FileTape(const std::string& filename,
                   const Delays& delays,
                   std::size_t memory_limit_bytes)
    : filename_(filename)
    , delays_(delays)
    , memory_limit_bytes_(memory_limit_bytes)
    {
    file_ = std::fopen(filename_.c_str(), "rb+");
    if (!file_) {
        throw std::runtime_error("Cannot open file: " + filename_);
    }
    
    if (std::fseek(file_, 0, SEEK_END) != 0) {
        throw std::runtime_error("Failed to seek end: " + filename_);
    }
    long end_pos = std::ftell(file_);
    if (end_pos < 0) {
        throw std::runtime_error("Failed to tell file size: " + filename_);
    }
    if (end_pos % CELL_SIZE != 0) {
        throw std::runtime_error("Invalid tape file size: " + filename_);
    }
    size_ = static_cast<std::size_t>(end_pos) / CELL_SIZE;

    std::fseek(file_, 0, SEEK_SET);
}

FileTape::~FileTape() {
    flushAndClearBuffer();
    if (file_) {
        std::fclose(file_);
    }

    if (is_temporary_) {
        std::remove(filename_.c_str());
    }
}

int32_t FileTape::Read() {
    applyDelay(delays_.read_ms);
    return getValue(position_);
}

void FileTape::Write(int32_t value) {
    getValue(position_) = value;
    buffer_dirty_ = true;

    applyDelay(delays_.write_ms);
}

bool FileTape::Next() {
    if (!shift(1)) {
        return false;
    }

    applyDelay(delays_.shift_ms);
    return true;
}

bool FileTape::Prev() {
    if (!shift(-1)) {
        return false;
    }

    applyDelay(delays_.shift_ms);
    return true;
}

bool FileTape::Rewind(std::ptrdiff_t offset) {
    if (!shift(offset)) {
        return false;
    }

    std::size_t delay_if_use_next = delays_.shift_ms * std::abs(offset);
    applyDelay(std::min(delays_.rewind_ms, delay_if_use_next));
    return true;
}

std::size_t FileTape::Size() const {
    return size_;
}

std::size_t FileTape::Position() const {
    return position_;
}

void FileTape::SetMemoryLimit(std::size_t bytes) {
    if (bytes < buffer_.size() * CELL_SIZE) {
        flushAndClearBuffer();

        buffer_start_ = 0;
        buffer_dirty_ = false;
    }
    memory_limit_bytes_ = bytes;
}

std::unique_ptr<Tape> FileTape::CreateTemporary(std::size_t size,
                                                 std::size_t buffer_bytes) const {
    std::string tmp_name = "tmp/" + makeTmpFilename();
    std::FILE* f = std::fopen(tmp_name.c_str(), "wb");
    if (!f) {
        throw std::runtime_error("Cannot create tmp file: " + tmp_name);
    }
    if (size > 0) {
        std::fseek(f, static_cast<long>(size * CELL_SIZE - 1), SEEK_SET);
        std::fputc(0, f);
    }
    std::fclose(f);
    auto tmp = std::make_unique<FileTape>(tmp_name, delays_, buffer_bytes);
    tmp->is_temporary_ = true;

    return tmp;
}

std::string FileTape::makeTmpFilename() const {
    std::ostringstream oss;
    oss << std::this_thread::get_id();
    std::string thread_id = oss.str();
    std::string base = filename_;
    for (char& c : base) {
        if (c == '/' || c == '\\' || c == ':' || c == '.') {
            c = '_';
        }
    }
    return base + "_" + thread_id + "_" + std::to_string(++tmp_counter_) + ".bin";
}

int32_t& FileTape::getValue(std::size_t index) {
    loadBuffer(index);
    return buffer_[index - buffer_start_];
}

bool FileTape::shift(std::ptrdiff_t offset) {
    position_ += offset;

    if (position_ < 0 || position_ >= size_) {
        position_ -= offset;
        return false;
    }

    return true;
}

void FileTape::applyDelay(std::size_t ms) const {
    if (ms > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }
}

void FileTape::flushAndClearBuffer() {
    if (!buffer_dirty_) {
        buffer_.clear();
        buffer_.shrink_to_fit();
        return;
    }

    std::fseek(file_, buffer_start_ * CELL_SIZE, SEEK_SET);
    std::fwrite(buffer_.data(), CELL_SIZE, buffer_.size(), file_);
    std::fflush(file_);

    buffer_dirty_ = false;

    buffer_.clear();
    buffer_.shrink_to_fit();
}

void FileTape::loadBuffer(std::size_t target_cell) {
    if (target_cell >= buffer_start_ &&
        target_cell < buffer_start_ + buffer_.size()) {
        return;
    }

    flushAndClearBuffer();

    std::size_t max_cells = memory_limit_bytes_ / CELL_SIZE;
    if (max_cells == 0) {
        max_cells = 1;
    }

    buffer_start_ = target_cell;

    std::size_t new_size = std::min(max_cells, size_ - buffer_start_);
    buffer_.resize(new_size);

    std::fseek(file_, buffer_start_ * CELL_SIZE, SEEK_SET);
    std::fread(buffer_.data(), CELL_SIZE, buffer_.size(), file_);

    buffer_dirty_ = false;
}