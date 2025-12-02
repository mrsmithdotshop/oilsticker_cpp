window.addEventListener('DOMContentLoaded', () => {
    // Auto-capitalize oil type field
    document.getElementById('oilType').addEventListener('input', (e) => {
        e.target.value = e.target.value.toUpperCase();
    });

    let selectedDevice = null;
    let devices = [];
    const canvas = document.getElementById('labelCanvas');
    const ctx = canvas.getContext('2d');
    const bgImage = new Image();

    // Set correct path to background image
    bgImage.src = 'images/default.png';
    bgImage.onload = drawLabel;

    function initBrowserPrint() {
        BrowserPrint.getDefaultDevice("printer", device => {
            selectedDevice = device;
            devices.push(device);

            BrowserPrint.getLocalDevices(
                deviceList => {
                    deviceList.forEach(d => {
                        if (!selectedDevice || d.uid !== selectedDevice.uid) devices.push(d);
                    });
                },
                () => alert("Could not get local Zebra printers."),
                "printer"
            );
        }, error => alert("BrowserPrint error: " + error));
    }

    function pad2(num) { return num.toString().padStart(2,'0'); }

    // -----------------------------------------------------
    // UPDATE CALCULATED FIELDS (with mileage null handling)
    // -----------------------------------------------------
    function updateCalculatedFields() {
        const today = new Date();
        document.getElementById('today').value =
            `${pad2(today.getMonth() + 1)}/${pad2(today.getDate())}/${String(today.getFullYear()).slice(-2)}`;

        const mileageRaw = document.getElementById('mileage').value.trim();
        const mileage = parseInt(mileageRaw);
        const nextService = parseInt(document.getElementById('nextService').value) || 0;

        // If mileage is missing → CLEAR and EXIT
        if (!mileageRaw || isNaN(mileage) || mileage <= 0) {
            document.getElementById('formattedMileage').value = "";
            document.getElementById('nextDate').value = "";
            return;
        }

        // Otherwise calculate formatted mileage
        document.getElementById('formattedMileage').value =
            (mileage + nextService).toLocaleString();

        // Calculate next date properly
        const nextDateObj = new Date();
        nextDateObj.setMonth(nextDateObj.getMonth() + 6);
        document.getElementById('nextDate').value =
            `${pad2(nextDateObj.getMonth() + 1)}/${pad2(nextDateObj.getDate())}/${String(nextDateObj.getFullYear()).slice(-2)}`;
    }

    // ----------------------
    // DRAW LABEL PREVIEW
    // ----------------------
    function drawLabel() {
        updateCalculatedFields();

        ctx.clearRect(0, 0, canvas.width, canvas.height);

        // White background
        ctx.fillStyle = "#ffffff";
        ctx.fillRect(0, 0, canvas.width, canvas.height);

        // Draw PNG background centered
        if (bgImage.complete && bgImage.naturalWidth !== 0) {
            const bgX = (canvas.width - bgImage.width) / 2;
            const bgY = (canvas.height - bgImage.height) / 2;
            ctx.drawImage(bgImage, bgX, bgY);
        }

        // 406x406 rounded outline
        const size = 406;
        const x = (canvas.width - size) / 2;
        const y = (canvas.height - size) / 2;
        const radius = 20;

        ctx.strokeStyle = "#000";
        ctx.lineWidth = 2;
        ctx.beginPath();
        ctx.moveTo(x + radius, y);
        ctx.lineTo(x + size - radius, y);
        ctx.quadraticCurveTo(x + size, y, x + size, y + radius);
        ctx.lineTo(x + size, y + size - radius);
        ctx.quadraticCurveTo(x + size, y + size, x + size - radius, y + size);
        ctx.lineTo(x + radius, y + size);
        ctx.quadraticCurveTo(x, y + size, x, y + size - radius);
        ctx.lineTo(x, y + radius);
        ctx.quadraticCurveTo(x, y, x + radius, y);
        ctx.stroke();

        // -----------------------------
        // Draw text
        // -----------------------------
        const oilType = document.getElementById('oilType').value;
        const todayText = document.getElementById('today').value;
        const formattedMileage = document.getElementById('formattedMileage').value;
        const nextDate = document.getElementById('nextDate').value;

        ctx.fillStyle = "#000";
        ctx.textBaseline = "top";

        // Top row (oil type + date)
        ctx.font = "15px Arial";
        const padding = 25;
        const textY1 = y + (285 * size / 406) - 20;

        ctx.fillText(oilType, x + padding, textY1);
        ctx.fillText(todayText, x + size - padding - ctx.measureText(todayText).width, textY1);

        // Bottom row (mileage + next date) — ONLY IF EXISTS
        ctx.font = "30px Arial";
        const textY2 = y + (365 * size / 406) - 25;

        if (formattedMileage) {
            ctx.fillText(formattedMileage, x + padding, textY2);
        }

        if (nextDate) {
            ctx.fillText(nextDate, x + size - padding - ctx.measureText(nextDate).width, textY2);
        }
    }

    // Redraw on form changes
    ['templateName','oilType','mileage','nextService'].forEach(id => {
        document.getElementById(id).addEventListener('input', drawLabel);
    });

    // -------------------------
    // PRINTING
    // -------------------------
    document.getElementById('printBtn').addEventListener('click', () => {
        if (!selectedDevice) {
            alert("No printer found");
            return;
        }

        const template = document.getElementById('templateName').value.toUpperCase();
        const oilType = document.getElementById('oilType').value;
        const today = document.getElementById('today').value;
        const formattedMileage = document.getElementById('formattedMileage').value;
        const nextDate = document.getElementById('nextDate').value;

        const zpl = `
^XA
^XF${template}^FS
^FN2^FD${oilType}^FS
^FN3^FD${today}^FS
${formattedMileage ? `^FN4^FD${formattedMileage}^FS` : ""}
${nextDate ? `^FN5^FD${nextDate}^FS` : ""}
^XZ
        `;

        selectedDevice.send(zpl, undefined, err => {
            if (err) alert("Printer Error: " + err);
        });

    // Clear all inputs except nextService
    ['oilType','mileage','today','formattedMileage','nextDate']
    .forEach(id => document.getElementById(id).value = "");

    // Redraw empty label
    drawLabel();

    });

    initBrowserPrint();
    drawLabel();

});

