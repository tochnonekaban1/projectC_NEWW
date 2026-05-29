#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

// максимальное количество заданий и машин (чтобы массив не переполнялся)
#define MAX_JOBS 100
#define MAX_MACHINES 20
#define INF 1000000000

// структура для задания
typedef struct 
{
    int id;           // номер задания
    int t;            // время выполнения
    int cnt;          // сколько допустимых машин
    int ok[MAX_MACHINES]; // список допустимых машин
} Job;

// структура для решения (куда назначили и какая нагрузка)
typedef struct 
{
    int where[MAX_JOBS];      // для каждого задания - на какой машине
    int load[MAX_MACHINES];   // нагрузка на каждую машину
    int makespan;             // максимальная нагрузка (самая загруженная машина)
    char name[50];            // имя алгоритма для вывода
} Solution;

Job jobs[MAX_JOBS]; // массив заданий
int n, m; // n - сколько заданий, m - сколько машин

// функция возвращает большее из двух чисел
int max(int a, int b) 
{
    return a > b ? a : b;
}

// пересчитываем makespan (максимальную нагрузку) для решения
void compute_makespan(Solution *sol) 
{
    sol->makespan = 0;
    for (int i = 0; i < m; i++) 
    {
        if (sol->load[i] > sol->makespan)
            sol->makespan = sol->load[i];
    }
}

// инициализируем решение (зануляем всё)
void init_solution(Solution *sol, const char *name) 
{
    for (int i = 0; i < MAX_JOBS; i++)
        sol->where[i] = -1; // -1 значит ещё не назначено
    for (int i = 0; i < MAX_MACHINES; i++)
        sol->load[i] = 0;
    sol->makespan = 0;
    
    int i = 0;
    while (name[i] != '\0' && i < 49) 
    {
        sol->name[i] = name[i];
        i++;
    }
    sol->name[i] = '\0';
}

// выводим решение на экран
void print_solution(const Solution *sol) 
{
    printf("\n=== %s ===\n", sol->name);
    printf("Назначения:\n");
    for (int i = 0; i < n; i++)
        printf("  задание %d (t=%d) -> машина %d\n", i, jobs[i].t, sol->where[i]);
    printf("Нагрузка:\n");
    for (int i = 0; i < m; i++)
        printf("  машина %d: %d\n", i, sol->load[i]);
    printf("Makespan = %d\n", sol->makespan);
}

// копируем одно решение в другое (нужно для локального поиска)
void copy_solution(Solution *dest, const Solution *src) 
{
    for (int i = 0; i < n; i++)
        dest->where[i] = src->where[i];
    for (int i = 0; i < m; i++)
        dest->load[i] = src->load[i];
    dest->makespan = src->makespan;
}

// сравнение для сортировки по убыванию времени выполнения задания
int cmp_desc(const void *a, const void *b) {
    Job *ja = (Job*)a;
    Job *jb = (Job*)b;
    return jb->t - ja->t; // по убыванию
}

// ==================== 1. Simple Greedy ====================
// идем по заданиям в том порядке, в котором они даны в файле
// каждое ставим на машину с наименьшей текущей нагрузкой среди допустимых
void simple_greedy(Solution *sol) 
{
    init_solution(sol, "Simple Greedy");
    
    for (int j = 0; j < n; j++) {
        int best = -1;
        int min_load = INF;
        // перебираем все машины, на которые можно поставить задание
        for (int i = 0; i < jobs[j].cnt; i++) 
        {
            int m = jobs[j].ok[i];
            if (sol->load[m] < min_load) 
            {
                min_load = sol->load[m];
                best = m;
            }
        }
        // если нашли подходящую машину, ставим
        if (best != -1) {
            sol->where[jobs[j].id] = best;
            sol->load[best] += jobs[j].t;
        }
    }
    compute_makespan(sol);
}

// ==================== 2. LPT Greedy ====================
// сначала сортируем задания по убыванию времени (самые большие первые)
// потом делаем то же самое - на самую свободную машину
void lpt_greedy(Solution *sol) {
    init_solution(sol, "LPT Greedy");
    
    // копируем задания в отдельный массив чтобы отсортировать
    Job sorted[MAX_JOBS];
    for (int i = 0; i < n; i++)
        sorted[i] = jobs[i];
    
    // сортируем по убыванию
    qsort(sorted, n, sizeof(Job), cmp_desc);
    
    for (int j = 0; j < n; j++) {
        int best = -1;
        int min_load = INF;
        
        for (int i = 0; i < sorted[j].cnt; i++) {
            int m = sorted[j].ok[i];
            if (sol->load[m] < min_load) {
                min_load = sol->load[m];
                best = m;
            }
        }
        
        if (best != -1) {
            sol->where[sorted[j].id] = best;
            sol->load[best] += sorted[j].t;
        }
    }
    compute_makespan(sol);
}

// ==================== 3. Multifit ====================
// проверяем, можно ли разместить все задания при ограничении C
// (жадно пытаемся положить каждое задание на любую машину, где влезет)
int can_pack(int C) {
    int load[MAX_MACHINES] = {0};
    Job sorted[MAX_JOBS];
    
    for (int i = 0; i < n; i++)
        sorted[i] = jobs[i];
    qsort(sorted, n, sizeof(Job), cmp_desc); // большие задания сначала
    
    for (int j = 0; j < n; j++) 
    {
        int placed = 0;
        // пытаемся поставить на любую допустимую машину
        for (int i = 0; i < sorted[j].cnt; i++) 
        {
            int m = sorted[j].ok[i];
            if (load[m] + sorted[j].t <= C) 
            {
                load[m] += sorted[j].t;
                placed = 1;
                break;
            }
        }
        if (!placed) return 0; // не влезло никуда
    }
    return 1; // всё влезло
}

// бинарный поиск минимального makespan
// потом просто берём LPT решение (потому что надо конкретное назначение вывести)
void multifit(Solution *sol) {
    init_solution(sol, "Multifit");
    
    int max_job = 0, sum = 0;
    for (int i = 0; i < n; i++) 
    {
        sum += jobs[i].t;
        if (jobs[i].t > max_job) max_job = jobs[i].t;
    }
    
    // левая граница - самое большое задание, правая - сумма всех
    int left = max_job, right = sum, best = right;
    while (left < right) {
        int mid = (left + right) / 2;
        if (can_pack(mid)) {
            best = mid;
            right = mid;
        } else {
            left = mid + 1;
        }
    }
    
    // выводим найденное значение, но само назначение берём из LPT
    lpt_greedy(sol);
    printf("\n[Multifit] найденный makespan = %d\n", best);
}

// ==================== 4. LP + Local Search ====================
// просто жадный алгоритм (2-приближение по идее Shmoys-Tardos)
void lp_approximation(Solution *sol) {
    init_solution(sol, "LP Approximation");
    
    Job sorted[MAX_JOBS];
    for (int i = 0; i < n; i++)
        sorted[i] = jobs[i];
    qsort(sorted, n, sizeof(Job), cmp_desc);
    
    for (int j = 0; j < n; j++) {
        int best = -1;
        int best_val = INF;
        
        for (int i = 0; i < sorted[j].cnt; i++) {
            int m = sorted[j].ok[i];
            int new_load = sol->load[m] + sorted[j].t;
            if (new_load < best_val) {
                best_val = new_load;
                best = m;
            }
        }
        
        if (best != -1) {
            sol->where[sorted[j].id] = best;
            sol->load[best] += sorted[j].t;
        }
    }
    compute_makespan(sol);
}

// локальный поиск: пытаемся перекинуть задание с самой загруженной машины на другую
// если это уменьшает makespan, то оставляем так и повторяем
void local_search(Solution *sol) 
{
    int improved = 1;
    
    while (improved) {
        improved = 0;
        compute_makespan(sol);
        
        // находим самую загруженную машину
        int busy = -1;
        for (int i = 0; i < m; i++) {
            if (sol->load[i] == sol->makespan) 
            {
                busy = i;
                break;
            }
        }
        if (busy == -1) break;
        
        // пробуем перекинуть задания с этой машины
        for (int j = 0; j < n; j++) {
            if (sol->where[j] != busy) continue;
            
            for (int k = 0; k < jobs[j].cnt; k++) {
                int to = jobs[j].ok[k];
                if (to == busy) continue;
                
                // запоминаем старые нагрузки
                int old_busy = sol->load[busy];
                int old_to = sol->load[to];
                
                // пробуем переназначить
                sol->load[busy] -= jobs[j].t;
                sol->load[to] += jobs[j].t;
                compute_makespan(sol);
                
                // если стало лучше, оставляем
                if (sol->makespan < old_busy) {
                    sol->where[j] = to;
                    improved = 1;
                    break;
                } else {
                    // возвращаем как было
                    sol->load[busy] = old_busy;
                    sol->load[to] = old_to;
                }
            }
            if (improved) break;
        }
    }
    compute_makespan(sol);
}

// ==================== Сравнение всех алгоритмов ====================
void compare_all() {
    Solution s[5];
    printf("СРАВНЕНИЕ АЛГОРИТМОВ\n");
    
    simple_greedy(&s[0]);      // простой жадный
    lpt_greedy(&s[1]);         // LPT жадный
    multifit(&s[2]);           // мультифит (бинарный поиск)
    lp_approximation(&s[3]);   // LP-приближение
    
    copy_solution(&s[4], &s[3]);
    local_search(&s[4]);       // применяем локальный поиск к LP решению
    
    // ищем лучший результат среди всех
    int best = INF;
    for (int i = 0; i < 5; i++)
        if (s[i].makespan < best) best = s[i].makespan;
    
    // выводим таблицу сравнения
    printf("\n%-25s %10s %10s\n", "Алгоритм", "Makespan", "Отношение");
    printf("------------------------------------------------\n");
    for (int i = 0; i < 5; i++) {
        printf("%-25s %8d %12.3f\n", 
               s[i].name, s[i].makespan, (double)s[i].makespan / best);
    }
    
    // выводим подробные решения
    for (int i = 0; i < 5; i++)
        print_solution(&s[i]);
}

// ==================== Чтение данных из файла ====================
int read_data(const char *filename) {
    FILE *f = stdin;
    if (filename != NULL) {
        f = fopen(filename, "r");
        if (!f) return 0; // если файл не открылся, возвращаем 0
    }
    
    fscanf(f, "%d %d", &n, &m); // сначала читаем количество заданий и машин
    
    for (int i = 0; i < n; i++) {
        jobs[i].id = i;
        fscanf(f, "%d %d", &jobs[i].t, &jobs[i].cnt);
        for (int j = 0; j < jobs[i].cnt; j++)
            fscanf(f, "%d", &jobs[i].ok[j]);
    }
    
    if (f != stdin) fclose(f);
    return 1;
}

// тестовый пример по умолчанию (если файл не задан)
void default_test() 
{
    printf("Используем тестовый пример\n");
    n = 8; m = 3;
    
    jobs[0] = (Job){0, 9, 2, {0, 1}};
    jobs[1] = (Job){1, 7, 2, {1, 2}};
    jobs[2] = (Job){2, 6, 2, {0, 2}};
    jobs[3] = (Job){3, 5, 3, {0, 1, 2}};
    jobs[4] = (Job){4, 4, 1, {1}};
    jobs[5] = (Job){5, 3, 2, {0, 2}};
    jobs[6] = (Job){6, 2, 3, {0, 1, 2}};
    jobs[7] = (Job){7, 8, 2, {1, 2}};
}

// ==================== main ====================
int main(int argc, char *argv[]) {
    printf("\nОбобщенная задача распределения нагрузки\n");
    
    // если передан аргумент командной строки, читаем из файла
    if (argc > 1) {
        if (!read_data(argv[1]))
            default_test(); // если файл не открылся, используем тест по умолчанию
    } 
    else 
    {
        default_test(); // иначе тоже тест по умолчанию
    }
    
    printf("Заданий: %d, Машин: %d\n", n, m);
    for (int i = 0; i < n; i++) {
        printf("Задание %d: t=%d, машины: ", i, jobs[i].t);
        for (int j = 0; j < jobs[i].cnt; j++)
            printf("%d ", jobs[i].ok[j]);
        printf("\n");
    }
    
    compare_all();
    
    return 0;
}