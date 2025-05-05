#include "tape.hpp"

#include <vector>

// Простая реализация Tape для тестирования: хранит данные в памяти, игнорирует ограничение по памяти
class VectorTape : public Tape {
  public:
    VectorTape(const std::vector<int32_t>& init)
        : data_(init), pos_(0) {}

    std::unique_ptr<Tape> CreateTemporary(std::size_t size, std::size_t) const override {
        return std::make_unique<VectorTape>(std::vector<int32_t>(size, 0));
    }

    int32_t Read() override {
        return data_.at(pos_);
    }

    void Write(int32_t value) override {
        if (pos_ >= data_.size()) data_.resize(pos_ + 1);
        data_[pos_] = value;
    }

    bool Next() override {
        if (pos_ + 1 > data_.size()) {
            return false;
        }
        ++pos_;
        return true;
    }

    bool Prev() override {
        if (pos_ == 0) {
            return false;
        }
        --pos_;
        return true;
    }

    bool Rewind(std::ptrdiff_t offset) override {
        auto new_pos = static_cast<std::ptrdiff_t>(pos_) + offset;
        if (new_pos < 0 || static_cast<std::size_t>(new_pos) > data_.size()) {
            return false;
        }
        pos_ = static_cast<std::size_t>(new_pos);
        return true;
    }

    std::size_t Size() const override { return data_.size(); }
    std::size_t Position() const override { return pos_; }
    void SetMemoryLimit(std::size_t /*bytes*/) override {}

private:
    std::vector<int32_t> data_;
    std::size_t pos_;
};