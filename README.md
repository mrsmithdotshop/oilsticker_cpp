Oil Change Label Generator

OilStickerApp is a lightweight macOS application for generating and printing professional oil-change reminder labels. It uses Qt (6.10.1) for both arm64 and x86_64 macs. The app is designed for use with 2×2" (406×406 px) thermal labels and the Zebra label printer using ZPL (Zebra Programming Language). It has been tested with a Zebra ZD420.

The application collects basic service information—oil brand & grade, current date, next service mileage/date—and sends it directly to the printer using the system’s standard CUPS lpr print command. A ZPL template stored on the printer itself handles the layout, so only the variable fields (mileage, date, oil type) are transmitted.

Using the Settings menu you can select any CUPS connected printer, select your own 448x418 (406x406) pixel PNG background image, and enter the ZPL template name stored on the label printer.

A built-in preview window shows the label with a customizable background image. Backgrounds can be designed or tested using tools such as the online Labelary ZPL viewer:
https://labelary.com/viewer.html

This tool provides a fast, reliable workflow for printing clean, consistent service labels in an automotive shop environment.

<img width="562" height="740" alt="samplelabel" src="https://github.com/user-attachments/assets/8e5da06f-daaa-4a5a-aa00-de8ec41dea98" />
