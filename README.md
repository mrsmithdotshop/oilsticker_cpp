Oil Change Label Generator

OilStickerApp is a lightweight macOS application for generating and printing professional oil-change reminder labels. The app is designed for use with 2×2" (406×406 px) thermal labels and the Zebra label printer using ZPL (Zebra Programming Language). It has been tested with a Zebra ZD420.

Now using Qt 6.10.1 for arm64 and x86_64 support.

The application collects basic service information—oil brand & grade, current date, next service mileage/date—and sends it directly to the printer using the system’s standard CUPS lpr print command. A ZPL template stored on the printer itself handles the layout, so only the variable fields (mileage, date, oil type) are transmitted.

Using the Settings menu you can select any CUPS connected printer, select your own 406x406 pixel PNG background image, and enter the ZPL template name stored on the label printer.

A built-in preview window shows the label with a customizable background image. Backgrounds can be designed or tested using tools such as the online Labelary ZPL viewer:
https://labelary.com/viewer.html

If saving the preview label image directly from the Zebra label printer it will be 448x418 even though the printer area is only 406x406. 

This tool provides a fast, reliable workflow for printing clean, consistent service labels in an automotive shop environment.

<img width="1224" height="1480" alt="sampleLabel" src="https://github.com/user-attachments/assets/09ebc534-5e07-445d-8297-5b1588c27a78" />
