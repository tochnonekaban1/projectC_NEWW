@echo off
echo ========================================
echo Запуск тестов для GAP
echo ========================================

echo.
echo Тест 1: простой пример
main.exe test1.txt

echo.
echo Тест 2: с ограничениями
main.exe test2.txt

echo.
echo Тест 3: большой пример
main.exe test3.txt

echo.
echo Тест 4: одно задание
main.exe test4.txt

echo.
echo Тест 5: все задания везде
main.exe test5.txt

echo.
echo Все тесты выполнены
pause