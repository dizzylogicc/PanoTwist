## PanoTwist
PanoTwist is a free, open source editor for fully spherical (360° x 180°) equirectangular panoramas. It can perform the following editing functions:
* Setting the initial viewpoint
* Rescaling the panorama
* Patching nadir and zenith:
  * With an image
  * With the average color under the patch
  * With a specific color

Panoramas can be manipulated one by one or as a batch.

Rescaling panoramas is sometimes needed as there can be hard limits on the maximum size; for example, in order to upload a panorama file to Facebook, it must be 6000 x 3000 px or smaller.

PanoTwist is a non-destructive editor in the sense that it leaves the original images unchanged so that it's always possible to go back. The edited files are saved separately in a subfolder.

64-bit windows installer is available from the [author's website](https://dizzylogic.com/panotwist).

## Getting started
This code is structured as a Visual Studio 2013 project. It has three dependencies which are not included into the distribution and need to be downloaded/compiled separately:
* [Qt5](https://www.qt.io/download) - user interface (uses Core, Gui, Widgets)
* [OpenCV](https://opencv.org/) - image manipulation
* [Exiv2](http://www.exiv2.org/download.html) - EXIF and XMP data editing

Exiv2, in turn, depends on Expat, XMP SDK and zlib, but those dependencies are usually packaged with Exiv2 itself.

So the steps needed to get this project up and running on your local machine are as follows:
* Clone this project: `git clone https://github.com/dizzylogicc/PanoTwist`
* Download [Qt5](https://www.qt.io/download), if you don't have it already. Downloading the binaries is much simpler than downloading the sources and compiling them. The binaries must match your Visual Studio version and the runtime library (multi-threaded, multi-threaded DLL, etc.).
* Download OpenCV and Exiv2. Here too, downloading binaries is simpler than downloading and compiling the sources. 
* Open the `PanoTwist.sln` file with Visual Studio 2013 (or later) with the [Qt plugin](http://doc.qt.io/archives/vs-addin/index.html) installed.
* Point the project to where Qt5 is installed (`Qt5 menu -> Qt Options` and `Qt5 menu -> Qt Project Settings`).
* Specify the include directories for Qt5, OpenCV and Exiv2 (`Project menu -> Properties -> Configuration Properties -> VC++ Directores -> Include directories`).
