#  WebRtc noise suppression

This project is a standalone noise suppression module which reference from google's official webrtc project. I rewraped it so that it can be used in other places. The files used in this project in sync with  official webrtc project respository. (branch de10eea, 2018.12).

**TODO List:**			

- [x] noise suppression in hight sample rate	- DONE supproted 32khz and 48 khz.
- [ ] Explaining the principle of Webrtc's noise suppression algorithm.  - TODO


The noise suppression process flow can be summed up as below:

![1544091468844](assets/1544091468844.png)



WebRtc uses AudioBuffer class to split audio date into different bands.	I have capture the main splitting filter function in order to support 32khz and 48khz.

For more details, please find https://chromium.googlesource.com/external/webrtc/+/master  and the codes.

------

## Update history:
[中文说明-for ver0.1](https://github.com/jagger2048/WebRtc_noise_suppression/readme_cn.md)

-  ver 1.0			2018-12-6
- [x] 1. Update the dependent files in sync with official webrtc project (branch de10eea, 2018.12).
- [x] 2. Wraped noise suppression interface and support 32khz,48khz sample rete.
- [x] 3. Mono supported.Stereo supported can be modify easily.
- ver 0.1			2018-7
- [x] 1. Capture the code of noise suppression module in webrtc project.
- [x] 2. A simple implement of noise suppression alorithm,supported 8khz and 16khz.
