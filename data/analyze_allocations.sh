#!/bin/bash

# Скрипт для анализа файлов xtmemory от Valgrind

if [ $# -eq 0 ]; then
    echo "Использование: $0 <файл.xtmemory.kcg>"
    echo "Или: $0 *.kcg для анализа всех файлов"
    exit 1
fi

for XTFILE in "$@"; do
    if [ ! -f "$XTFILE" ]; then
        echo "Файл не найден: $XTFILE"
        continue
    fi

    BASENAME=$(basename "$XTFILE" .kcg)
    echo "==============================================="
    echo "Анализ файла: $XTFILE"
    echo "==============================================="

    # Конвертируем в текстовый формат
    callgrind_annotate --auto=yes "$XTFILE" > "${BASENAME}_analysis.txt"

    # Извлекаем топ-10 функций по использованию памяти
    echo ""
    echo "Топ-10 функций по выделенной памяти:"
    grep -A 20 "Events annotated" "${BASENAME}_analysis.txt" | \
        grep -E "^[[:space:]]*[0-9,]+" | head -10

    # Суммарная статистика
    echo ""
    echo "Общая статистика:"
    grep -E "Total[[:space:]]+[0-9,]+|Events[[:space:]]+[0-9,]+" "${BASENAME}_analysis.txt"

    echo ""
    echo "Детальный отчет сохранен в: ${BASENAME}_analysis.txt"
    echo "Для просмотра: less ${BASENAME}_analysis.txt"
done