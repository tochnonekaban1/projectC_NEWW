#!/bin/bash
echo "Запуск тестов для GAP"

echo ""
echo "Тест 1: простой пример"
./main test1.txt

echo ""
echo "Тест 2: с ограничениями"
./main test2.txt

echo ""
echo "Тест 3: большой пример"
./main test3.txt

echo ""
echo "Тест 4: одно задание"
./main test4.txt

echo ""
echo "Тест 5: все задания везде"
./main test5.txt

echo ""
echo "Все тесты выполнены"