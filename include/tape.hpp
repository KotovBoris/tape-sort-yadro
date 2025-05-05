#pragma once

#include <cstddef>
#include <cstdint>

#include <memory>

class Tape {
protected:
    Tape() = default;
public:
    virtual ~Tape() = default;

    // Чтение и запись по текущей позиции
    virtual int32_t Read() = 0;
    virtual void Write(int32_t value) = 0;

    // Сдвиги: на одну ячейку и перемотка
    // Если перемещение невозможно, возвращают false и ничего не делают
    virtual bool Next() = 0;
    virtual bool Prev() = 0;
    virtual bool Rewind(std::ptrdiff_t offset) = 0;

    // Позиция и размер (в элементах)
    virtual std::size_t Size() const = 0;
    virtual std::size_t Position() const = 0;

    // Сброс к началу
    virtual void Reset() {
        Rewind(-static_cast<std::ptrdiff_t>(Position()));
    }

    // Устанавливает лимит памяти (для кешей/буферов) в байтах
    virtual void SetMemoryLimit(std::size_t bytes) = 0;

    // Создать временную ленту с указанным размером и буфером
    virtual std::unique_ptr<Tape> CreateTemporary(std::size_t size, std::size_t buffer_bytes) const = 0;

    // Запрещаем копирование
    Tape(const Tape&) = delete;
    Tape& operator=(const Tape&) = delete;
    Tape(Tape&&) = default;
    Tape& operator=(Tape&&) = default;
};
