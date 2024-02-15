## PanoTwist
![PanoTwist interface](img/PanoTwist_Interface.png?raw=true "PanoTwist interface")

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

## Patching nadir or zenith
When shots for equirectangular panoramas are taken, the area directly underneath the camera (known as nadir) is usually not photographed since it contains the tripod or other equipment. Here is the nadir of a typical panorama where the black area corresponds to parts of the space not captured by camera shots:
![Unpatched nadir](img/UnpatchedNadir.png?raw=true "Unpatched nadir")

PanoTwist can patch the nadir (or zenith) with an image chosen by the user or a given color. After patching up the image above:
![Patched nadir](img/PatchedNadir.png?raw=true "Patched nadir")

## Windows Installer

A 64-bit windows installer is provided in the **Win64_Installer** folder. To install, simply download and run the provided **PanoTwist.msi** installer file.

## Building from source
This code is structured as a Visual Studio 2013 project. It has three dependencies which are not included into the distribution and need to be downloaded/compiled separately:
* [Qt5](https://www.qt.io/download) - user interface (uses Core, Gui, Widgets)
* [OpenCV](https://opencv.org/) - image manipulation
* [Exiv2](http://www.exiv2.org/download.html) - EXIF and XMP data editing

Exiv2, in turn, depends on Expat, XMP SDK and zlib, but those dependencies are usually packaged with Exiv2 itself.

Therefore the steps needed to get this project up and running on your local machine with Visual Studio are as follows:
* Clone this project: `git clone https://github.com/dizzylogicc/PanoTwist`
* Download [Qt5](https://www.qt.io/download), if you don't have it already. Downloading Qt5 binaries is much simpler than downloading the sources and compiling them. The binaries must match your Visual Studio version and the runtime library (multi-threaded, multi-threaded DLL, etc.).
* Download OpenCV and Exiv2. Here too, downloading the binary files is simpler than downloading and compiling the sources. 
* Open the `PanoTwist.sln` file with Visual Studio 2013 (or later) with the [Qt plugin](http://doc.qt.io/archives/vs-addin/index.html) installed.
* Point the project to where Qt5 is located on your system (`Qt5 menu -> Qt Options` and `Qt5 menu -> Qt Project Settings`).
* Specify the include directories for Qt5, OpenCV and Exiv2 (`Project menu -> Properties -> Configuration Properties -> VC++ Directores -> Include directories`).
* Add the following libraries for OpenCV and Exiv2 support to the project (`Project -> Add existing item`): libexiv2.lib, libexpat.lib, opencv_world310.lib, xmpsdk.lib, zlib1.lib. Note that library names may differ somewhat on your system. If these libraries are already among the project files, remove them from the project first.
* Build the project!

## License
This project is licensed under GNU General Public License v.3 (GNU GPL v.3) or later version.

## Acknowledgements
This project uses some neat code from demottea's [CvImageWidget](https://github.com/delmottea/QtOpenCVWidget). Hat tip for making it publicly available!
