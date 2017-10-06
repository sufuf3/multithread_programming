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
```sh
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
  
# 只產生一個 thread，並讓 thread 在執行過程中切換 core 執行。  
## 執行結果  
## gprof ./raytracing | less  
## CPU  
## out.ppm  
  
# 產生兩個 threads，讓兩個 threads 分別在不同 core 上執行。  
## 執行結果  
```shell=
$ make run
./raytracing
# Rendering scene
Done!
Execution time of raytracing() : 25.096402 sec
```
## gprof ./raytracing | less  
```shell=
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
## CPU  
![](https://i.imgur.com/PyPXhJS.png)  
## out.ppm  
![](https://i.imgur.com/e1cOstx.png)  
  
# 產生兩個 threads，讓兩個 threads 分別在同個 core 上執行。  
## 執行結果  
## gprof ./raytracing | less  
## CPU  
## out.ppm  
  
  
Ref: https://hackmd.io/s/rJbdOYQc-  
https://hackmd.io/s/rks802qob  
https://hackmd.io/s/rJSc2rtoZ  
