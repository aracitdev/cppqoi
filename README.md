
[![Logo](https://qoiformat.org/qoi-logo.svg)](https://qoiformat.org/)

# cppqoi
[![Language](https://img.shields.io/badge/language-C++-blue.svg)](https://isocpp.org/)

 A c++ header only QOI (quite okay image format) implementation.  This implementation does not support streaming image data, however it likely will in the future.

 ## API

 There are no plans to change the signatures of already existing functions, however future functionality (such as streaming image data) may be added.

 ### Encoding
```cpp
cppqoi::QoiFile file(rawImageData, imageDataWidth, imageDataHeight, imageDataChannels, imageDataColorSpace);
cppqoi::WriteQoi("myfile.qoi", file);
```

 ### Decoding
```cpp
cppqoi::QoiFile file;
cppqoi::LoadQoi("myfile.qoi", file);
```
 # License
 cppqoi is licensed under the [MIT](LICENSE) license.
