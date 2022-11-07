# lcm_test

```
https://lcm-proj.github.io/tut_lcmgen.html
```
Defining a data type - example_t 참고하기

Let's define an example type called example_t. Create a file called example_t.lcm with the following contents.
```
package exlcm;
struct example_t
{
    int64_t  timestamp;
    double   position[3];
    double   orientation[4]; 
    int32_t  num_ranges;
    int16_t  ranges[num_ranges];
    string   name;
    boolean  enabled;
}
```

이 폴더에서는 map_data_t.lcm, example_t.lcm를 참고하면 된다. 
위와 같이 lcm 파일을 생성하고 
C++	-> lcm-gen -x example_t.lcm
lcm-gen -x *.lcm (* lcm 이름 달린 모든 것을 전부 생성한다.) 
< map_data_t.lcm, example_t.lcm 파일이 전부 생성된다.>

```
https://lcm-proj.github.io/tut_cpp.html
```
생성된 파일을 실행하기 위해 LCM_TEST_SIMPLE 의 mainwindow.h에 #include 를 통해 
