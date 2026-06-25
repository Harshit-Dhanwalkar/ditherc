# DitherC

dither.c uses the Stucki kernel and the cyclic row selector

```bash
make
./build/dither -k atkinson > img_atkinson.h
./build/dither -k stucki > img_stucki.h
./build/dither -k floyd   > img_floyd.h
./build/dither -k jarvis > img_jarvis.h
./build/dither -k burkes > img_burkes.h
./build/dither -k sierra > img_sierra.h
```

| **original input**  | <img src="./images/in.bmp" width="100">               | <img src="./images/in2.bmp" width="100">               | <img src="./images/in3.bmp" width="100">               | <img src="./images/in4.bmp" width="100">               |
| :------------------ | :---------------------------------------------------- | :----------------------------------------------------- | :----------------------------------------------------- | :----------------------------------------------------- |
| **atkinson output** | <img src="./results/out_in_atkinson.bmp" width="100"> | <img src="./results/out_in2_atkinson.bmp" width="100"> | <img src="./results/out_in3_atkinson.bmp" width="100"> | <img src="./results/out_in4_atkinson.bmp" width="100"> |
| **burkes output**   | <img src="./results/out_in_burkes.bmp" width="100">   | <img src="./results/out_in2_burkes.bmp" width="100">   | <img src="./results/out_in3_burkes.bmp" width="100">   | <img src="./results/out_in4_burkes.bmp" width="100">   |
| **floyd output**    | <img src="./results/out_in_floyd.bmp" width="100">    | <img src="./results/out_in2_floyd.bmp" width="100">    | <img src="./results/out_in3_floyd.bmp" width="100">    | <img src="./results/out_in4_floyd.bmp" width="100">    |
| **jarvis output**   | <img src="./results/out_in_jarvis.bmp" width="100">   | <img src="./results/out_in2_jarvis.bmp" width="100">   | <img src="./results/out_in3_jarvis.bmp" width="100">   | <img src="./results/out_in4_jarvis.bmp" width="100">   |
| **sierra output**   | <img src="./results/out_in_sierra.bmp" width="100">   | <img src="./results/out_in2_sierra.bmp" width="100">   | <img src="./results/out_in3_sierra.bmp" width="100">   | <img src="./results/out_in4_sierra.bmp" width="100">   |
| **stucki output**   | <img src="./results/out_in_stucki.bmp" width="100">   | <img src="./results/out_in2_stucki.bmp" width="100">   | <img src="./results/out_in3_stucki.bmp" width="100">   | <img src="./results/out_in4_stucki.bmp" width="100">   |
