#pragma once

#include "delays.hpp"
#include "tape.hpp"

#include <cstdint>
#include <cstdio>

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

// Эмуляция Tape через файл с контролируемым буфером и лимитом памяти
class FileTape : public Tape {
public:
    explicit FileTape(const std::string& filename,
                      const Delays& delays,
                      std::size_t memory_limit_bytes = 0);
    ~FileTape() override;

    int32_t Read() override;
    void Write(int32_t value) override;

    bool Next() override;
    bool Prev() override;
    bool Rewind(std::ptrdiff_t offset) override;

    std::size_t Size() const override;
    std::size_t Position() const override;

    void SetMemoryLimit(std::size_t bytes) override;
    std::unique_ptr<Tape> CreateTemporary(std::size_t size,
                                          std::size_t buffer_bytes) const override;

  private:
    std::string makeTmpFilename() const;  // Уникальное имя для временной ленты
    int32_t& getValue(std::size_t index); // Получить значение по индексу 
    bool shift(std::ptrdiff_t offset);    // Сдвинуть position_
    void applyDelay(std::size_t ms) const;
    void flushAndClearBuffer();
    // Обновляет буффер, если target_cell в него не попадает
    void loadBuffer(std::size_t target_cell);

    std::FILE* file_ = nullptr;
    std::string filename_;
    std::size_t size_ = 0; // размер файла (и ленты, соответственно)
    std::ptrdiff_t position_ = 0;

    Delays delays_;
    std::size_t memory_limit_bytes_ = 0;

    std::vector<int32_t> buffer_;
    std::size_t buffer_start_ = 0; // индекс первой буфферизированной ячейки
    bool buffer_dirty_ = false;    // нужно ли будет flush-ить

    bool is_temporary_ = false;    // нужно ли удалить файл

    static const std::size_t CELL_SIZE; // размер 1 ячейки
    static std::size_t tmp_counter_;    // для makeTmpFilename 
};