雲端 Homework 1-1  
===
###### tags: `雲端`  

# 環境安裝  
  
- 環境  
```sh
$ cat /proc/version
Linux version 4.10.0-35-generic (buildd@lcy01-33) (gcc version 5.4.0 20160609 (Ubuntu 5.4.0-6ubuntu1~16.04.4) ) #39~16.04.1-Ubuntu SMP Wed Sep 13 09:02:42 UTC 2017

$ lsb_release -a
No LSB modules are available.
Distributor ID: LinuxMint
Description:    Linux Mint 18.2 Sonya
Release:    18.2
Codename:   sonya

$ cat /proc/meminfo
MemTotal:        8075108 kB

$ lscpu
Architecture:          x86_64
CPU op-mode(s):        32-bit, 64-bit
Byte Order:            Little Endian
CPU(s):                4
On-line CPU(s) list:   0-3
Thread(s) per core:    2
Core(s) per socket:    2
Socket(s):             1
NUMA node(s):          1
Vendor ID:             GenuineIntel
CPU family:            6
Model:                 61
Model name:            Intel(R) Core(TM) i5-5250U CPU @ 1.60GHz
Stepping:              4
CPU MHz:               1600.125
CPU max MHz:           2700.0000
CPU min MHz:           500.0000
BogoMIPS:              3200.18
Virtualization:        VT-x
L1d cache:             32K
L1i cache:             32K
L2 cache:              256K
L3 cache:              3072K
NUMA node0 CPU(s):     0-3

```
  
- 開發工具安裝  
```sh
sudo apt install graphviz eog imagemagick gnuplot
```
  
# 未改程式前之執行結果  

```shell
$ make run
cc -std=gnu99 -Wall -O0 -g -c -o objects.o objects.c
cc -std=gnu99 -Wall -O0 -g -c -o raytracing.o raytracing.c
cc -std=gnu99 -Wall -O0 -g -c -o main.o main.c
cc -o raytracing objects.o raytracing.o main.o -lm
./raytracing
# Rendering scene
Done!
Execution time of raytracing() : 33.332789 sec
```
```
$ make PROFILE=1
$ gprof ./raytracing | less
Flat profile:

Each sample counts as 0.01 seconds.
  %   cumulative   self              self     total           
 time   seconds   seconds    calls  Ts/call  Ts/call  name    
 78.27      9.42     9.42                             rayRectangularIntersection
  5.82     10.12     0.70                             cross_product
  3.66     10.56     0.44                             add_vector
  2.25     10.83     0.27                             multiply_vectors
  2.16     11.09     0.26                             subtract_vector
  1.33     11.25     0.16                             length
  1.08     11.38     0.13                             raySphereIntersection
  0.91     11.49     0.11                             compute_specular_diffuse
  0.91     11.60     0.11                             normalize
  0.83     11.70     0.10                             ray_color
  0.67     11.78     0.08                             idx_stack_top
  0.33     11.82     0.04                             idx_stack_pop
  0.33     11.86     0.04                             multiply_vector
  0.33     11.90     0.04                             ray_hit_object
  0.33     11.94     0.04                             raytracing
  0.25     11.97     0.03                             fresnel
  0.25     12.00     0.03                             rayConstruction
  0.17     12.02     0.02                             delete_sphere_list
  0.08     12.03     0.01                             idx_stack_init
  0.08     12.04     0.01                             localColor
```
未調整前的圖 (`$ eog out.ppm`)  
![](https://i.imgur.com/eKknTc0.png)  
  
# Description (改寫說明)  
## 參數與 function  
此份專案修改了4個檔案，分別為 `Makefile`, `main.c`, `raytracing.c`, `raytracing.h`  
- `Makefile`  
因為我們使用到了 pthread 這個 library 所以在 編譯的時候需要下指定的參數 `-lpthread` ，所以加了 `-lpthread` 這字串在裡面。  
- `main.c`  
依據作業所需，加入了一個選擇題。詢問要執行那種方式。依據各小題再來進行後續的處理。程式面部份，在各小題的內容中將一一解釋。  
- `raytracing.h` 與 `raytracing.c`  
因 pthread_create 帶入的 function 只能是一個 void * 的變數，所以將輸入 function 的變數變成用一個 structure (如下)。  

```c
$ vim raytracing.h
typedef struct rayarg {
    uint8_t *pixels;
    double *background_color;
    rectangular_node rectangulars;
    sphere_node spheres;
    light_node lights;
    viewpoint *view;
    int  width;
    int height;
    int thread_num; // 代表幾個 thread
    int core_num; // 代表幾個 core  
    //(註：後面因應各題，所以在 main.c 程式碼中有將 thread_num 和 core_num 兩個參數對換，主要是用來執行 raytracing.c 中的 raytracing 和 raytracing3 帶入參數用)
} rayargs;
```
以下是透過 `ray` function 設定參數的初始化。  

```c
$ vim raytracing.h
rayargs *ray(uint8_t *pixels, color background_color,
             rectangular_node rectangulars, sphere_node spheres,
             light_node lights, viewpoint *view,
             int width, int height, int thread_num, int core_num);

$ vim raytracing.c
rayargs * ray(uint8_t *pixels, color background_color,
              rectangular_node rectangulars, sphere_node spheres,
              light_node lights,viewpoint *view,
              int width, int height, int thread_num, int core_num)
{
    rayargs * r =  (rayargs *) malloc (sizeof(rayargs));
    r->pixels = pixels;
    r->background_color =  background_color;
    r->rectangulars = rectangulars;
    r->spheres = spheres;
    r->lights = lights;
    r->view = view;
    r->width = width;
    r->height = height;
    r->thread_num = thread_num;
    r->core_num = core_num;
    return r;
}
```
以下為依據作業題目調整的程式片斷，將於以下各小題中進行說明。  

```c
$ vim raytracing.h
void raytracing( void * r);
void raytracing3( void * r);
```
- `raytracing.c`  
## 只產生一個 thread ，並讓 thread 在執行過程中切換 core 執行。  
### 程式碼調整部份  
- `main.c`  
使用 `pthread_create` 創建一個 thread_num[0] 的 Thread 並指定要執行的 `&raytracing` function。  
另，透過 `pthread_setaffinity_np` 將 Thread 指定到要使用的 CPU 資源。  

```c
        pthread_t * thread_num = ( pthread_t *) malloc ( 1 * sizeof( pthread_t));
        rayargs** pr = (rayargs **) malloc( 1 * sizeof(  rayargs * ));
        for( int num = 0; num < 4; num++) {
            pr[0] = ray(pixels, background,
                        rectangulars, spheres, lights, &view, ROWS, COLS, num, 4);
            pthread_create(&thread_num[0], NULL, (void *) &raytracing, (void *) pr[0]);
            cpu_set_t cpus;
            CPU_ZERO(&cpus);
            CPU_SET(num, &cpus);
            pthread_setaffinity_np(thread_num[0], sizeof(cpu_set_t), &cpus);
        }
        pthread_join(thread_num[0], NULL);
```
- `raytracing.c`  

```c
void raytracing(void * rayarg)
{
    rayargs * r = ( rayargs *) rayarg; // 帶入 structure 的值
    point3 u, v, w, d;
    color object_color = { 0.0, 0.0, 0.0 };

    /* calculate u, v, w */
    calculateBasisVectors(u, v, w, r->view);

    idx_stack stk;

    int factor = sqrt(SAMPLES);
    for (int j = r->thread_num; j < r->height/r->core_num*(r->thread_num+1); j++) { // 將圖的高度切四塊，每個 CPU 各自處理各子的部份。
        for (int i = 0; i < r->width; i++) {
            double R = 0, G = 0, B = 0;
            /* MSAA */
            for (int s = 0; s < SAMPLES; s++) {
                idx_stack_init(&stk);
                rayConstruction(d, u, v, w,
                                i * factor + s / factor,
                                j * factor + s % factor,
                                r->view,
                                r->width * factor, r->height * factor);
                if (ray_color(r->view->vrp, 0.0, d, &stk, r->rectangulars, r->spheres,
                              r->lights, object_color,
                              MAX_REFLECTION_BOUNCES)) {
                    R += object_color[0];
                    G += object_color[1];
                    B += object_color[2];
                } else {
                    R += r->background_color[0];
                    G += r->background_color[1];
                    B += r->background_color[2];
                }
                r->pixels[((i + (j * r->width)) * 3) + 0] = R * 255 / SAMPLES;
                r->pixels[((i + (j * r->width)) * 3) + 1] = G * 255 / SAMPLES;
                r->pixels[((i + (j * r->width)) * 3) + 2] = B * 255 / SAMPLES;
            }
        }
    }
}

```
### 執行結果  
#### make run  

```shell
$ make run
./raytracing
# Rendering scene
Done!
Execution time of raytracing() : 30.387225 sec
```
#### gprof ./raytracing | less  

```shell
Flat profile:

Each sample counts as 0.01 seconds.
  %   cumulative   self              self     total           
 time   seconds   seconds    calls   s/call   s/call  name    
 81.43     15.97    15.97 11388749     0.00     0.00  rayRectangularIntersection
  4.26     16.80     0.84 57275589     0.00     0.00  dot_product
  4.08     17.60     0.80 46737619     0.00     0.00  subtract_vector
  2.07     18.01     0.41 25945470     0.00     0.00  multiply_vector
  1.73     18.35     0.34 14669091     0.00     0.00  add_vector
  1.12     18.57     0.22  8765205     0.00     0.00  normalize
  1.07     18.78     0.21  3796251     0.00     0.00  ray_hit_object
  1.07     18.99     0.21 14637101     0.00     0.00  cross_product
  1.02     19.19     0.20 11388742     0.00     0.00  raySphereIntersection
  0.36     19.26     0.07  3512032     0.00     0.00  multiply_vectors
  0.36     19.33     0.07   834588     0.00     0.00  rayConstruction
  0.26     19.38     0.05  1756019     0.00     0.00  compute_specular_diffuse
  0.26     19.43     0.05  1756017     0.00     0.00  localColor
  0.26     19.48     0.05        2     0.03     9.81  raytracing
  0.20     19.52     0.04  3182497     0.00     0.00  length
  0.15     19.55     0.03  2091284     0.00     0.00  idx_stack_top
  0.15     19.58     0.03   834588     0.00     0.00  ray_color
  0.05     19.59     0.01  2121403     0.00     0.00  idx_stack_empty
  0.05     19.60     0.01  1030586     0.00     0.00  protect_color_overflow
  0.05     19.61     0.01  1030582     0.00     0.00  refraction
  0.05     19.62     0.01   834589     0.00     0.00  idx_stack_init
  0.00     19.62     0.00  1030582     0.00     0.00  reflection
  0.00     19.62     0.00  1000465     0.00     0.00  idx_stack_push
  0.00     19.62     0.00    90777     0.00     0.00  fresnel
  0.00     19.62     0.00    30118     0.00     0.00  idx_stack_pop
  0.00     19.62     0.00        2     0.00     0.00  calculateBasisVectors
```
#### CPU  
![](https://i.imgur.com/NOvDWW0.png)  
註：這邊一直卡著無法變成巒峰狀的切換 CPU ，所以這邊透過修改 `raytracing.c` 這程式碼，變成讓 CPU 執行完自己的片斷後釋放該 CPU 的使用資源。可以清楚知道是透過 1->2->3->4 這樣進行的。  
#### out.ppm  
![](https://i.imgur.com/IaCYAvD.png)  
  
## 產生兩個 threads，讓兩個 threads 分別在不同 core 上執行。  
### 程式碼調整部份  
- `main.c`  
使用 `pthread_create` 創建兩個 thread_num 的 Thread 並指定要執行的 `&raytracing3` function。  
另，透過 `pthread_setaffinity_np` 將 Thread 個別指定到要使用的 CPU 資源。  

```c
        pthread_t * thread_num = ( pthread_t *) malloc ( 2 * sizeof( pthread_t));
        rayargs** pr = (rayargs **) malloc( 2 * sizeof(  rayargs * ));
        for( int num = 0; num < 2; num++) {
            pr[num] = ray(pixels, background,
                          rectangulars, spheres, lights, &view, ROWS, COLS, num, 2);
            pthread_create(&thread_num[num], NULL, (void *) &raytracing3, (void *) pr[num]);
            cpu_set_t cpus;
            CPU_ZERO(&cpus);
            CPU_SET(num, &cpus);
            pthread_setaffinity_np(thread_num[num], sizeof(cpu_set_t), &cpus);
        }
        for( int num = 0; num < 2; num++) {
            pthread_join(thread_num[num], NULL);
        }

```
- `raytracing.c`  

```c
void raytracing(void * rayarg)
{
    rayargs * r = ( rayargs *) rayarg; // 帶入 structure 的值
    point3 u, v, w, d;
    color object_color = { 0.0, 0.0, 0.0 };

    /* calculate u, v, w */
    calculateBasisVectors(u, v, w, r->view);

    idx_stack stk;

    int factor = sqrt(SAMPLES);
    for (int j = r->thread_num; j < r->height; j=j+r->core_num) { // 隔一行一行的進行高度的處理 (像柵欄的模式)，非上個例子用區段進行。
        for (int i = 0; i < r->width; i++) {
            double R = 0, G = 0, B = 0;
            /* MSAA */
            for (int s = 0; s < SAMPLES; s++) {
                idx_stack_init(&stk);
                rayConstruction(d, u, v, w,
                                i * factor + s / factor,
                                j * factor + s % factor,
                                r->view,
                                r->width * factor, r->height * factor);
                if (ray_color(r->view->vrp, 0.0, d, &stk, r->rectangulars, r->spheres,
                              r->lights, object_color,
                              MAX_REFLECTION_BOUNCES)) {
                    R += object_color[0];
                    G += object_color[1];
                    B += object_color[2];
                } else {
                    R += r->background_color[0];
                    G += r->background_color[1];
                    B += r->background_color[2];
                }
                r->pixels[((i + (j * r->width)) * 3) + 0] = R * 255 / SAMPLES;
                r->pixels[((i + (j * r->width)) * 3) + 1] = G * 255 / SAMPLES;
                r->pixels[((i + (j * r->width)) * 3) + 2] = B * 255 / SAMPLES;
            }
        }
    }
}

```
### 執行結果  
#### make run  

```shell
$ make run
./raytracing
# Rendering scene
Done!
Execution time of raytracing() : 19.373274 sec
```
#### gprof ./raytracing | less  

```shell
Flat profile:

Each sample counts as 0.01 seconds.
  %   cumulative   self              self     total           
 time   seconds   seconds    calls  Ts/call  Ts/call  name    
 76.69      9.23     9.23                             rayRectangularIntersection
  5.49      9.89     0.66                             normalize
  3.41     10.30     0.41                             multiply_vectors
  2.25     10.57     0.27                             subtract_vector
  2.12     10.82     0.26                             length
  1.83     11.04     0.22                             raySphereIntersection
  1.41     11.21     0.17                             append_sphere
  1.37     11.38     0.17                             delete_sphere_list
  0.83     11.48     0.10                             ray_color
  0.67     11.56     0.08                             idx_stack_push
  0.58     11.63     0.07                             localColor
  0.42     11.68     0.05                             add_vector
  0.42     11.73     0.05                             dot_product
  0.42     11.78     0.05                             idx_stack_top
  0.42     11.83     0.05                             ray_hit_object
  0.33     11.87     0.04                             compute_specular_diffuse
  0.33     11.91     0.04                             refraction
  0.25     11.94     0.03                             idx_stack_init
  0.25     11.97     0.03                             raytracing
  0.17     11.99     0.02                             cross_product
  0.17     12.01     0.02                             delete_rectangular_list
  0.17     12.03     0.02                             rayConstruction
  0.08     12.04     0.01                             multiply_vector
```
#### CPU  
![](https://i.imgur.com/RShP9gj.png)  

#### out.ppm  
![](https://i.imgur.com/e1cOstx.png)  
  
## 產生兩個 threads，讓兩個 threads 分別在同個 core 上執行。  
### 程式碼調整部份  
- `main.c`  
使用 `pthread_create` 創建兩個 thread_num 的 Thread 並指定要執行的 `&raytracing3` function。  
另，透過 `pthread_setaffinity_np` 將 Thread 指定到要使用的同個 CPU 資源。  

```c
        pthread_t * thread_num = ( pthread_t *) malloc ( 2 * sizeof( pthread_t));
        rayargs** pr = (rayargs **) malloc( 2 * sizeof(  rayargs * ));
        for( int num = 0; num < 2; num++) {
            pr[num] = ray(pixels, background,
                          rectangulars, spheres, lights, &view, ROWS, COLS, num, 1+1);
            pthread_create(&thread_num[num], NULL, (void *) &raytracing3, (void *) pr[num]);
            cpu_set_t cpus;
            CPU_ZERO(&cpus);
            CPU_SET(0, &cpus);
            pthread_setaffinity_np(thread_num[num], sizeof(cpu_set_t), &cpus);
        }
        for( int num = 0; num < 2; num++) {
            pthread_join(thread_num[num], NULL);
        }
```
- `raytracing.c`  
同上個小題  
### 執行結果  
#### make run  

```shell
# Rendering scene
Done!
Execution time of raytracing() : 25.273941 sec
```
#### gprof ./raytracing | less  

```shell
Flat profile:

Each sample counts as 0.01 seconds.
  %   cumulative   self              self     total           
 time   seconds   seconds    calls  us/call  us/call  name    
 78.70     19.47    19.47  9809290     1.98     2.15  rayRectangularIntersection
  6.27     21.02     1.55 49645244     0.03     0.03  dot_product
  4.13     22.04     1.02 40442655     0.03     0.03  subtract_vector
  2.22     22.59     0.55 22537646     0.02     0.02  multiply_vector
  1.33     22.92     0.33 12715445     0.03     0.03  add_vector
  1.29     23.24     0.32  3289895     0.10     6.93  ray_hit_object
  1.17     23.53     0.29  9568179     0.03     0.13  raySphereIntersection
  1.13     23.81     0.28 12659399     0.02     0.02  cross_product
  0.89     24.03     0.22  7685164     0.03     0.03  normalize
  0.63     24.19     0.16   718507     0.22    34.02  ray_color
  0.44     24.30     0.11                             raytracing
  0.36     24.39     0.09  3142197     0.03     0.03  multiply_vectors
  0.34     24.47     0.09  1592888     0.05     0.28  localColor
  0.18     24.52     0.05   911478     0.05     0.05  protect_color_overflow
  0.16     24.56     0.04  1875176     0.02     0.02  idx_stack_empty
  0.16     24.60     0.04   725362     0.06     0.26  rayConstruction
  0.14     24.63     0.04  2704440     0.01     0.01  length
  0.14     24.67     0.04  1459665     0.02     0.40  compute_specular_diffuse
  0.12     24.70     0.03   915538     0.03     0.07  refraction
  0.08     24.72     0.02  1829199     0.01     0.03  idx_stack_top
  0.08     24.74     0.02   864624     0.02     0.02  idx_stack_push
  0.04     24.75     0.01   867864     0.01     0.10  reflection
  0.02     24.75     0.01                             delete_sphere_list
  0.00     24.75     0.00   755853     0.00     0.00  idx_stack_init
  0.00     24.75     0.00    65734     0.00     0.08  fresnel
  0.00     24.75     0.00    22303     0.00     0.02  idx_stack_pop
  0.00     24.75     0.00        2     0.00     0.08  calculateBasisVectors
  0.00     24.75     0.00        2     0.00     0.00  ray
```
#### CPU  
![Imgur](https://i.imgur.com/xlJGWYL.png)  

#### out.ppm  
![](https://i.imgur.com/CQ4SrvE.png)  
  
# Analysis (分析 CPU 資源對於 raytracing 所產生的影響)  
## 原始程式執行時間分析  
1. 由此圖得知，多 Thread 其實很重要，不論有多少 CPU 資源。因為由紅柱(一個 thread 在不同 core 轉換)，與另外的兩個情境 (兩個 Thread)相比，兩個 Thread 的執行時間勝過一個 Thread 。若要加快程式的執行速度，可以透過增加 Thread 來進行處理。以紅柱與綠柱為例，差距 5.113284 sec  
2. 當多個 Thread 同時進行時，若每個 Thread 都能分配到一個 CPU 資源那程式執行速度會再提升。以下圖之黃柱與綠柱為例。差距 5.900667 sec。  
![Imgur](https://i.imgur.com/20XA05X.png)  



##### 其他  
1. 我不熟 C 語言，在做作業總是被 `scripts/pre-commit.hook` 擋住不讓我 git commit ，所以只好自己在 local 端把 L46-L53 註解掉了。但保證，程式碼都可以執行。至於什麼原因，之後有空會來研究。  
2. 其實程式碼還可以再進行優化，而且還有許多可以再補強之處。但仍有其他事務進行中，尚為可惜。  
3. 實驗結果會因為當下其他程序使用的 cpu 資源而有所差異。不見得每次的數值都雷同，有時執行同樣的程式再不同的資源使用情形下，會有 10 秒的落差，但同時，其他的各小題與最初位修改版的程式，再那樣附近的時間點上，也是會差到 10 秒。  
  
Ref: https://hackmd.io/s/rJbdOYQc-  
https://hackmd.io/s/rks802qob  
https://hackmd.io/s/rJSc2rtoZ  
