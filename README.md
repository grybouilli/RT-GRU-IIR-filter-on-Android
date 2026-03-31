<!-- Improved compatibility of back to top link: See: https://github.com/othneildrew/Best-README-Template/pull/73 -->
<a id="readme-top"></a>
<!--
*** Thanks for checking out the Best-README-Template. If you have a suggestion
*** that would make this better, please fork the repo and create a pull request
*** or simply open an issue with the tag "enhancement".
*** Don't forget to give the project a star!
*** Thanks again! Now go create something AMAZING! :D
-->



<!-- PROJECT SHIELDS -->
<!--
*** I'm using markdown "reference style" links for readability.
*** Reference links are enclosed in brackets [ ] instead of parentheses ( ).
*** See the bottom of this document for the declaration of the reference variables
*** for contributors-url, forks-url, etc. This is an optional, concise syntax you may use.
*** https://www.markdownguide.org/basic-syntax/#reference-style-links
-->
[![Contributors][contributors-shield]][contributors-url]
[![Forks][forks-shield]][forks-url]
[![Stargazers][stars-shield]][stars-url]
[![Issues][issues-shield]][issues-url]
[![project_license][license-shield]][license-url]
[![LinkedIn][linkedin-shield]][linkedin-url]



<!-- PROJECT LOGO -->
<br />
<div align="center">
  <a href="https://github.com/grybouilli/RT-lowpass-filter-on-Android">
    <!-- <img src="images/logo.png" alt="Logo" width="80" height="80"> -->
  </a>

<h3 align="center">Real-Time Lowpass Filter on Android</h3>

  <p align="center">
    A GRU-based lowpass filter applied to real-time audio on Android using liboboe and ONNX RT 
    <br />
    <a href="https://github.com/grybouilli/RT-lowpass-filter-on-Android"><strong>Explore the docs »</strong></a>
    <br />
    <br />
    <a href="https://github.com/grybouilli/RT-lowpass-filter-on-Android">View Demo</a>
    &middot;
    <a href="https://github.com/grybouilli/RT-lowpass-filter-on-Android/issues/new?labels=bug&template=bug-report---.md">Report Bug</a>
    &middot;
    <a href="https://github.com/grybouilli/RT-lowpass-filter-on-Android/issues/new?labels=enhancement&template=feature-request---.md">Request Feature</a>
  </p>
</div>



<!-- TABLE OF CONTENTS -->
<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#dependencies">Dependencies</a></li>
        <li><a href="#building">Building</a></li>
        <li><a href="#running">Running</a></li>
      </ul>
    </li>
    <li><a href="#roadmap">Roadmap</a></li>
    <li><a href="#contributing">Contributing</a></li>
    <li><a href="#license">License</a></li>
    <li><a href="#contact">Contact</a></li>
  </ol>
</details>



<!-- ABOUT THE PROJECT -->
## About The Project

<!-- [![Product Name Screen Shot][product-screenshot]](https://example.com) -->

This repo uses the model from the project [First-Order Lowpass GRU](https://github.com/grybouilli/first-order-lowpass-GRU).

In this repo, you'll find :
* inference code based on ONNX Runtime to filter real-time audio;
* audio stream manipulation to pass Android input stream into the filter and then into the Android output stream. 

The stream manipulations are done with [liboboe](github.com/google/oboe).

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- GETTING STARTED -->
## Getting Started
### Dependencies

Needed:
* CMake 
* [Android NDK (tested with version r27d)](https://developer.android.com/ndk/downloads?hl=fr)
* adb (to export to your Android device)
  
Fetched by cmake :
* [Oboe v1.9.0](github.com/google/oboe)
* [ONNX Runtime v1.24.2](https://github.com/csukuangfj/onnxruntime-libs/releases/download/v1.24.2/)
### Building
After installing the android ndk at `/path/to/ndk`, you clone the repo and build with cmake :
```sh
git clone https://github.com/grybouilli/RT-lowpass-filter-on-Android
cmake -B build -DCMAKE_TOOLCHAIN_FILE=/path/to/ndk/build/cmake/android.toolchain.cmake [-GNinja] .
cmake --build build
```

### Running

Enable the developers options on your Android device. Connect it to your computer and allow adb in the developer's options. On your laptop, run :
```sh
adb kill-server
adb start-server
adb devices 
# at this point, you might get prompted on your android device to allow your machine 
adb push build/filtered /data/local/tmp
adb shell
> device:/ $ cd /data/local/tmp # the folder is hidden, auto complete doesn't work
> device:/data/local/tmp: $ ./filtered [frequency in Hz] 
```


<!-- ROADMAP -->
## Roadmap
- [x] Finish README.md
- [ ] Improve latency
- [ ] Implement Second-Order Lowpass 
- [ ] Try other inference engines
    - [ ] RTNeural
    - [ ] TF Lite
    - [ ] Executorch
- [ ] Try with [Anira](https://github.com/anira-project/anira)

See the [open issues](https://github.com/grybouilli/RT-lowpass-filter-on-Android/issues) for a full list of proposed features (and known issues).

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- CONTRIBUTING -->
## Contributing

Contributions are what make the open source community such an amazing place to learn, inspire, and create. Any contributions you make are **greatly appreciated**.

If you have a suggestion that would make this better, please fork the repo and create a pull request. You can also simply open an issue with the tag "enhancement".
Don't forget to give the project a star! Thanks again!

1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the Branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

<p align="right">(<a href="#readme-top">back to top</a>)</p>

### Top contributors:

<a href="https://github.com/grybouilli/RT-lowpass-filter-on-Android/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=grybouilli/RT-lowpass-filter-on-Android" alt="contrib.rocks image" />
</a>



<!-- LICENSE -->
## License

Distributed under the MIT License. See `LICENSE.md` for more information.

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- CONTACT -->
## Contact

Nicolas Gry - grybouilli at outlook.fr

Project Link: [https://github.com/grybouilli/RT-lowpass-filter-on-Android](https://github.com/grybouilli/RT-lowpass-filter-on-Android)

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->
[contributors-shield]: https://img.shields.io/github/contributors/grybouilli/RT-lowpass-filter-on-Android.svg?style=for-the-badge
[contributors-url]: https://github.com/grybouilli/RT-lowpass-filter-on-Android/graphs/contributors
[forks-shield]: https://img.shields.io/github/forks/grybouilli/RT-lowpass-filter-on-Android.svg?style=for-the-badge
[forks-url]: https://github.com/grybouilli/RT-lowpass-filter-on-Android/network/members
[stars-shield]: https://img.shields.io/github/stars/grybouilli/RT-lowpass-filter-on-Android.svg?style=for-the-badge
[stars-url]: https://github.com/grybouilli/RT-lowpass-filter-on-Android/stargazers
[issues-shield]: https://img.shields.io/github/issues/grybouilli/RT-lowpass-filter-on-Android.svg?style=for-the-badge
[issues-url]: https://github.com/grybouilli/RT-lowpass-filter-on-Android/issues
[license-shield]: https://img.shields.io/github/license/grybouilli/RT-lowpass-filter-on-Android.svg?style=for-the-badge
[license-url]: https://github.com/grybouilli/RT-lowpass-filter-on-Android/blob/main/LICENSE.md
[linkedin-shield]: https://img.shields.io/badge/-LinkedIn-black.svg?style=for-the-badge&logo=linkedin&colorB=555
[linkedin-url]: https://linkedin.com/in/nicolas-gry
[product-screenshot]: images/screenshot.png
