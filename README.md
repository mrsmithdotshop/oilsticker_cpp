Oil Change Label Generator

OilStickerApp is a lightweight macOS application for generating and printing professional oil-change reminder labels. It uses Qt (6.10.1) for both arm64 and x86_64 macs. The app is designed for use with 2×2" (406×406 px) thermal labels and the Zebra label printer using ZPL (Zebra Programming Language). It has been tested with a Zebra ZD420.

The application collects basic service information—oil brand & grade, current date, next service mileage/date—and sends it directly to the printer using the system’s standard CUPS lpr print command. A ZPL template stored on the printer itself handles the layout, so only the variable fields (mileage, date, oil type) are transmitted.

Using the Settings menu you can select any CUPS connected printer, select your own 448x418 (406x406) pixel PNG background image, and enter the ZPL template name stored on the label printer.

A built-in preview window shows the label with a customizable background image. Backgrounds can be designed or tested using tools such as the online Labelary ZPL viewer: https://labelary.com/viewer.html The label preview image ideally should be generated on the Zebra label printer itself to ensure the most accurate representation.

This tool provides a fast, reliable workflow for printing clean, consistent service labels in an automotive shop environment.

<img width="562" height="740" alt="samplelabel" src="https://github.com/user-attachments/assets/8e5da06f-daaa-4a5a-aa00-de8ec41dea98" />

The prorgram populates and sends four field name variables to the printer using ^FN in ZPL. See this doc page for details on using ^FN:
https://docs.zebra.com/us/en/printers/software/zpl-pg/c-zpl-zpl-commands/r-zpl-fn.html

Currently ^FN2, ^FN3, ^FN4, and ^FN5 are defined as:

        "^FN2^FD%2^FS\n" // oil type
        "^FN3^FD%3^FS\n" // today's date
        "^FN4^FD%4^FS\n" // next mileage
        "^FN5^FD%5^FS\n" // next date
