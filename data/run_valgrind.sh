#!/bin/bash

# Скрипт для детального анализа аллокаций памяти с Valgrind
# Автор: AI Assistant
# Дата: $(date)

echo "==============================================="
echo "Запуск детального анализа памяти с Valgrind"
echo "==============================================="

# Проверяем, существует ли исполняемый файл
if [ ! -f "./lab3" ]; then
    echo "Ошибка: файл ./lab3 не найден!"
    echo "Сначала соберите проект: cmake .. && make"
    exit 1
fi

# Проверяем, установлен ли Valgrind
if ! command -v valgrind &> /dev/null; then
    echo "Ошибка: Valgrind не установлен!"
    echo "Установите: sudo apt-get install valgrind"
    exit 1
fi

# Создаем уникальное имя для лог-файла
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
LOGFILE="valgrind_${TIMESTAMP}.log"
MEMCHECK_FILE="memcheck_${TIMESTAMP}.txt"

echo "Лог Valgrind: $LOGFILE"
echo "Файл с аллокациями: $MEMCHECK_FILE"
echo ""

# Функция для обработки Ctrl+C
cleanup() {
    echo ""
    echo "==============================================="
    echo "Анализ прерван пользователем"
    echo "Собранные данные сохранены в:"
    echo "  - $LOGFILE (полный лог Valgrind)"
    echo "  - $MEMCHECK_FILE (только аллокации)"
    echo "==============================================="
    exit 0
}

trap cleanup INT

echo "Запуск Valgrind с детальной трассировкой аллокаций..."
echo "Это может занять некоторое время..."
echo ""

# Запускаем Valgrind с максимальной детализацией
valgrind \
    --tool=memcheck \
    --leak-check=full \
    --show-leak-kinds=all \
    --track-origins=yes \
    --trace-children=yes \
    --keep-stacktraces=alloc-and-free \
    --num-callers=40 \
    --read-var-info=yes \
    --verbose \
    --log-file="$LOGFILE" \
    --xtree-memory=full \
    --xtree-memory-file="xtmemory_${TIMESTAMP}.kcg" \
    --time-stamp=yes \
    ./lab3

RETVAL=$?

echo ""
echo "Valgrind завершил работу с кодом: $RETVAL"
echo "Обрабатываю данные..."

# Создаем заголовок файла с аллокациями
{
echo "=== ДЕТАЛЬНАЯ ИНФОРМАЦИЯ О ВСЕХ АЛЛОКАЦИЯХ ПАМЯТИ ==="
echo "Файл: $MEMCHECK_FILE"
echo "Дата: $(date)"
echo "Программа: ./lab3"
echo "Время запуска Valgrind: $(grep -m 1 '^--' "$LOGFILE" 2>/dev/null | cut -d' ' -f2-)"
echo "Код завершения Valgrind: $RETVAL"
echo "===================================================="
echo ""
} > "$MEMCHECK_FILE"

# Проверяем, создался ли лог-файл
if [ ! -f "$LOGFILE" ]; then
    echo "Ошибка: Лог-файл $LOGFILE не создан!" | tee -a "$MEMCHECK_FILE"
    exit 1
fi

# Собираем статистику
TOTAL_ALLOCS=$(grep -c "^--[0-9]\+-- malloc\|^--[0-9]\+-- calloc\|^--[0-9]\+-- realloc" "$LOGFILE" 2>/dev/null || echo "0")
TOTAL_FREES=$(grep -c "^--[0-9]\+-- free" "$LOGFILE" 2>/dev/null || echo "0")

{
echo "=== СТАТИСТИКА ==="
echo "Всего аллокаций: $TOTAL_ALLOCS"
echo "Всего освобождений: $TOTAL_FREES"
echo ""

echo "=== ПОЛНЫЙ ЖУРНАЛ ОПЕРАЦИЙ С ПАМЯТЬЮ ==="
echo "Формат: [ВРЕМЯ] ОПЕРАЦИЯ(РАЗМЕР) = АДРЕС [СТЕК ВЫЗОВОВ]"
echo ""
} >> "$MEMCHECK_FILE"

# Обрабатываем каждую операцию с памятью
LINE_NUM=1
while IFS= read -r line; do
    # Ищем операции malloc/calloc/realloc
    if echo "$line" | grep -q "^--[0-9]\+-- \(malloc\|calloc\|realloc\|free\)"; then
        # Извлекаем временную метку (если есть)
        TIMESTAMP_INFO=""
        if [[ "$line" =~ ^--[0-9]+--\ ([0-9]+\.[0-9]+): ]]; then
            TIMESTAMP_INFO="[${BASH_REMATCH[1]}s] "
        fi

        # Извлекаем детали операции
        if echo "$line" | grep -q "malloc\|calloc\|realloc"; then
            # Для malloc/calloc/realloc
            OP=$(echo "$line" | sed -n 's/.*-- \(malloc\|calloc\|realloc\).*/\1/p')
            SIZE=$(echo "$line" | sed -n 's/.*(\([^)]*\)).*/\1/p')
            ADDR=$(echo "$line" | grep -o "= 0x[0-9a-fA-F]\+")

            echo "${TIMESTAMP_INFO}${OP^^}($SIZE) $ADDR" >> "$MEMCHECK_FILE"

            # Читаем следующие строки для стека вызовов
            for ((i=1; i<=10; i++)); do
                NEXT_LINE_NUM=$((LINE_NUM + i))
                NEXT_LINE=$(sed -n "${NEXT_LINE_NUM}p" "$LOGFILE" 2>/dev/null)

                if [ -z "$NEXT_LINE" ]; then
                    break
                fi

                if echo "$NEXT_LINE" | grep -q "^--[0-9]\+--     by 0x"; then
                    # Очищаем строку от префикса Valgrind
                    CLEAN_LINE=$(echo "$NEXT_LINE" | sed 's/^--[0-9]\+--\s*//')
                    # Пытаемся извлечь информацию о функции и файле
                    FUNC=$(echo "$CLEAN_LINE" | sed -n 's/.*: \(.*\) (.*/\1/p')
                    FILE=$(echo "$CLEAN_LINE" | sed -n 's/.* (\(.*\))$/\1/p')

                    if [ -n "$FUNC" ] && [ -n "$FILE" ]; then
                        echo "    → $FUNC ($FILE)" >> "$MEMCHECK_FILE"
                    elif [ -n "$CLEAN_LINE" ]; then
                        echo "    → $CLEAN_LINE" >> "$MEMCHECK_FILE"
                    fi
                elif echo "$NEXT_LINE" | grep -q "^--[0-9]\+--" || [ -z "$NEXT_LINE" ]; then
                    break
                fi
            done
            echo "" >> "$MEMCHECK_FILE"

        elif echo "$line" | grep -q "free"; then
            # Для free
            ADDR=$(echo "$line" | sed -n 's/.*(\(.*\)).*/\1/p')
            echo "${TIMESTAMP_INFO}FREE($ADDR)" >> "$MEMCHECK_FILE"

            # Читаем стек вызовов для free
            for ((i=1; i<=10; i++)); do
                NEXT_LINE_NUM=$((LINE_NUM + i))
                NEXT_LINE=$(sed -n "${NEXT_LINE_NUM}p" "$LOGFILE" 2>/dev/null)

                if [ -z "$NEXT_LINE" ]; then
                    break
                fi

                if echo "$NEXT_LINE" | grep -q "^--[0-9]\+--     by 0x"; then
                    CLEAN_LINE=$(echo "$NEXT_LINE" | sed 's/^--[0-9]\+--\s*//')
                    FUNC=$(echo "$CLEAN_LINE" | sed -n 's/.*: \(.*\) (.*/\1/p')
                    FILE=$(echo "$CLEAN_LINE" | sed -n 's/.* (\(.*\))$/\1/p')

                    if [ -n "$FUNC" ] && [ -n "$FILE" ]; then
                        echo "    → $FUNC ($FILE)" >> "$MEMCHECK_FILE"
                    elif [ -n "$CLEAN_LINE" ]; then
                        echo "    → $CLEAN_LINE" >> "$MEMCHECK_FILE"
                    fi
                elif echo "$NEXT_LINE" | grep -q "^--[0-9]\+--" || [ -z "$NEXT_LINE" ]; then
                    break
                fi
            done
            echo "" >> "$MEMCHECK_FILE"
        fi
    fi
    ((LINE_NUM++))
done < "$LOGFILE"

# Добавляем информацию о потерянной памяти
{
echo ""
echo "=== УТЕЧКИ ПАМЯТИ ==="
} >> "$MEMCHECK_FILE"

if grep -q "All heap blocks were freed" "$LOGFILE" 2>/dev/null; then
    echo "✓ Утечек памяти не обнаружено!" >> "$MEMCHECK_FILE"
else
    # Извлекаем информацию об утечках
    {
    grep -A 20 "LEAK SUMMARY:" "$LOGFILE" 2>/dev/null || echo "Информация об утечках не найдена"
    echo ""
    echo "Детали утечек:"
    grep -B 5 -A 5 "definitely lost\|indirectly lost\|possibly lost" "$LOGFILE" 2>/dev/null || echo "Нет детальной информации"
    } >> "$MEMCHECK_FILE"
fi

# Добавляем информацию об ошибках
{
echo ""
echo "=== ОШИБКИ ДОСТУПА К ПАМЯТИ ==="
} >> "$MEMCHECK_FILE"

ERRORS=$(grep -c "ERROR SUMMARY:" "$LOGFILE" 2>/dev/null || echo "0")
if [ "$ERRORS" -gt 0 ]; then
    grep -A 5 "ERROR SUMMARY:" "$LOGFILE" 2>/dev/null >> "$MEMCHECK_FILE"
else
    echo "✓ Ошибок доступа к памяти не обнаружено" >> "$MEMCHECK_FILE"
fi

# Создаем сводку по использованию памяти
{
echo ""
echo "=== ИСПОЛЬЗОВАНИЕ ПАМЯТИ ==="
} >> "$MEMCHECK_FILE"

grep -A 5 "HEAP SUMMARY:" "$LOGFILE" 2>/dev/null >> "$MEMCHECK_FILE"

# Анализируем использование памяти по функциям
{
echo ""
echo "=== АЛЛОКАЦИИ ПО ФУНКЦИЯМ ==="
echo "Функции, отсортированные по количеству аллокаций:"
echo ""
} >> "$MEMCHECK_FILE"

# Создаем временный файл для анализа
TEMP_FILE=$(mktemp)
grep -B 2 "by 0x" "$LOGFILE" 2>/dev/null | grep -E "(malloc|calloc|realloc|free)" -A 2 | grep "by 0x" | \
    sed 's/.*by 0x[^:]*: //' | sed 's/ (.*)//' | sort | uniq -c | sort -rn > "$TEMP_FILE" 2>/dev/null

if [ -s "$TEMP_FILE" ]; then
    while read -r line; do
        if [ -n "$line" ]; then
            echo "$line" >> "$MEMCHECK_FILE"
        fi
    done < "$TEMP_FILE"
else
    echo "Не удалось собрать статистику по функциям" >> "$MEMCHECK_FILE"
fi

rm -f "$TEMP_FILE" 2>/dev/null

# Добавляем информацию о top-10 аллокациях по размеру
{
echo ""
echo "=== ТОП-10 АЛЛОКАЦИЙ ПО РАЗМЕРУ ==="
echo ""
} >> "$MEMCHECK_FILE"

# Извлекаем размеры аллокаций
grep "^--[0-9]\+-- malloc\|^--[0-9]\+-- calloc\|^--[0-9]\+-- realloc" "$LOGFILE" 2>/dev/null | \
    sed 's/.*(//; s/).*//' | sort -nr | head -10 | while read -r size; do
    echo "$size байт" >> "$MEMCHECK_FILE"
done

echo ""
echo "==============================================="
echo "Анализ завершен!"
echo ""
echo "Созданы файлы:"
echo "  1. $LOGFILE      - полный лог Valgrind"
echo "  2. $MEMCHECK_FILE - детальная информация об аллокациях"
echo "  3. xtmemory_${TIMESTAMP}.kcg - данные для визуализации"
echo ""
echo "Размеры файлов:"
ls -lh "$LOGFILE" "$MEMCHECK_FILE" "xtmemory_${TIMESTAMP}.kcg" 2>/dev/null | awk '{print $5, $9}' || echo "Некоторые файлы не созданы"
echo ""
echo "Для просмотра:"
echo "  less $MEMCHECK_FILE"
echo "  или"
echo "  head -100 $MEMCHECK_FILE"
echo ""
echo "Для визуализации используйте:"
echo "  callgrind_annotate --auto=yes xtmemory_${TIMESTAMP}.kcg 2>/dev/null | head -50"
echo "==============================================="

# Даем возможность сразу посмотреть результаты
echo ""
read -p "Показать первые 50 строк memcheck файла? (y/N): " -n 1 -r
echo ""
if [[ $REPLY =~ ^[Yy]$ ]]; then
    echo "=== Первые 50 строк $MEMCHECK_FILE ==="
    head -50 "$MEMCHECK_FILE"
fi