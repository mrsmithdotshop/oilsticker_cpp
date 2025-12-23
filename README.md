Oil Change Label Generator

OilStickerApp is a lightweight application for generating and printing professional oil-change reminder labels. It now can print key tag labels and multiple 6 line labels (1"x2"). It uses Qt (6.10.1) for both Mac (arm64/x86_64) and Windows (x86_64). The app is designed for use with 2"×2" (406×406 px) thermal or pre-printed labels, and 1"x2" thermal labels on a second printer. Uses the Zebra ZPL (Zebra Programming Language). It has been tested with a Zebra ZD420 and GX420T. In my enviroment I am using one Zebra printer for the 2"x2" labels and a second Zebra printer for the 1"x2" key tag labels. The single printer using a folded keytag label can still be utilized by using the appropriate ZPL templates.

The application collects basic service information—oil brand & grade, current date, next service mileage/date—and sends it directly to the printer using the system’s standard CUPS lpr print command. Windows uses IPP. A ZPL template stored on the printer itself handles the layout, so only the variable fields (mileage, date, oil type) are transmitted.

Using the Settings menu you can select any CUPS connected printer or IPP printer IP address, select your own 448x418 (406x406) pixel PNG background image, and enter the ZPL template name stored on the label printer.

A built-in preview window shows the label with a customizable background image. Backgrounds can be designed or tested using tools such as the online Labelary ZPL viewer:
https://labelary.com/viewer.html

It is recommended production backgrounds be generated directly on the Zebra printer iteself via the web interface. This will generate the most accurate representation of the actual printed label. The image.png will be 448x418 which represents the entire label with backing not just the printed area of the 406x406 label. Currently the application is expecting a 448x418 PNG image.

The program populates Field Number (^FN) variables that are sent to the printer. For more information see: https://docs.zebra.com/us/en/printers/software/zpl-pg/c-zpl-zpl-commands/r-zpl-fn.html

There are three ZPL templates found in the zpl/ folder. DEFAULT.ZPL is used to print the service sticker. KEYTAG.ZPL is used to print key tag labels. LABEL.ZPL is used to print labels, two per label when cut in half.

This tool provides a fast, reliable workflow for printing clean, consistent service labels in an automotive shop environment.

Default Service Label 2"x2":
<img width="1200" height="1496" alt="oilsticker1" src="https://github.com/user-attachments/assets/9f2f2397-1be2-469e-88dc-67dba8556055" />

Keytag 1"x2" label:
<img width="1200" height="1496" alt="oilsticker2" src="https://github.com/user-attachments/assets/a10797ab-4f7d-4aef-ab71-f17e5410ee6c" />

