window.addEventListener('DOMContentLoaded', () => {

    // ------------------------
    //  AUTOCAPITALIZE OIL TYPE
    // ------------------------
    document.getElementById('oilType').addEventListener('input', (e) => {
        e.target.value = e.target.value.toUpperCase();
    });

    let selectedDevice = null;
    let devices = [];
    const canvas = document.getElementById('labelCanvas');
    const ctx = canvas.getContext('2d');
    const bgImage = new Image();

    // current style
    let currentStyle = "style1";

    // form field sets
    const oilFields = ['templateName','oilType','mileage','nextService'];
    const keyTagFields = ['customer','car','plate','vin','color','repairOrder'];

    // Force uppercase on KEY TAG fields
    keyTagFields.forEach(id => {
       const input = document.getElementById(id);
       if(input) {
           input.addEventListener('input', e => {
           e.target.value = e.target.value.toUpperCase();
           });
        }
    });

    const styleBackgrounds = {
        style1: "images/style1.png",
        style2: "images/style2.png"
    };

    // Load initial background
    bgImage.src = styleBackgrounds.style1;
    bgImage.onload = drawLabel;

    // ------------------------
    //  STYLE SWITCHER
    // ------------------------
    document.getElementById('labelStyle').addEventListener('change', (e) => {
        currentStyle = e.target.value;

        // Show/hide form fields
        if(currentStyle === 'style1') {
            document.getElementById('keyTagFields').style.display = 'none';
            oilFields.forEach(id => document.getElementById(id).parentNode.style.display = 'block');
            document.getElementById('templateName').value = "DEFAULT.ZPL";
        } else {
            document.getElementById('keyTagFields').style.display = 'block';
            oilFields.forEach(id => document.getElementById(id).parentNode.style.display = 'none');
            document.getElementById('templateName').value = "KEYTAG.ZPL";
        }

        bgImage.src = styleBackgrounds[currentStyle];
        drawLabel();
    });

    // ------------------------
    //  BROWSERPRINT INIT
    // ------------------------
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

    // ------------------------------
    // UPDATE CALCULATED MILEAGE ETC.
    // ------------------------------
    function updateCalculatedFields() {
        const today = new Date();
        document.getElementById('today').value =
            `${pad2(today.getMonth()+1)}/${pad2(today.getDate())}/${String(today.getFullYear()).slice(-2)}`;

        const mileageRaw = document.getElementById('mileage').value.trim();
        const mileage = parseInt(mileageRaw);
        const nextService = parseInt(document.getElementById('nextService').value) || 0;

        if (!mileageRaw || isNaN(mileage) || mileage <= 0) {
            document.getElementById('formattedMileage').value = "";
            document.getElementById('nextDate').value = "";
            return;
        }

        document.getElementById('formattedMileage').value =
            (mileage + nextService).toLocaleString();

        const nextDateObj = new Date();
        nextDateObj.setMonth(nextDateObj.getMonth() + 6);
        document.getElementById('nextDate').value =
            `${pad2(nextDateObj.getMonth()+1)}/${pad2(nextDateObj.getDate())}/${String(nextDateObj.getFullYear()).slice(-2)}`;
    }

    // ------------------------
    // DRAW PREVIEW
    // ------------------------
    function drawLabel() {
        updateCalculatedFields();

        ctx.clearRect(0, 0, canvas.width, canvas.height);
        ctx.fillStyle = "#fff";
        ctx.fillRect(0, 0, canvas.width, canvas.height);

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

        ctx.fillStyle = "#000";
        ctx.textBaseline = "top";

        if(currentStyle === "style1") {
            // OIL SERVICE layout
            const oilType = document.getElementById('oilType').value;
            const todayText = document.getElementById('today').value;
            const formattedMileage = document.getElementById('formattedMileage').value;
            const nextDate = document.getElementById('nextDate').value;

            ctx.font = "15px Arial";
            const padding = 25;
            const textY1 = y + (285 * size / 406) - 20;
            ctx.fillText(oilType, x + padding, textY1);
            ctx.fillText(todayText, x + size - padding - ctx.measureText(todayText).width, textY1);

            ctx.font = "30px Arial";
            const textY2 = y + (365 * size / 406) - 25;
            if(formattedMileage) ctx.fillText(formattedMileage, x + padding, textY2);
            if(nextDate) ctx.fillText(nextDate, x + size - padding - ctx.measureText(nextDate).width, textY2);
        } else {
//            // KEY TAG layout
//            ctx.font = "18px Arial";
//            let lineY = y + 15;
//            keyTagFields.forEach(id => {
//                const val = document.getElementById(id).value;
//                ctx.fillText(`${val}`, x + 30, lineY);
//                lineY += 30;
//            });
//        }
	// KEY TAG layout
	ctx.font = "18px Arial";
	let lineY = y + 15;

	keyTagFields.forEach((id, idx) => {
    		const val = document.getElementById(id).value;

    	// Normal small text line
    	ctx.fillText(val, x + 30, lineY);
    	lineY += 30;

    	// If this is the last field (repairOrder), print it big too
    	if (idx === keyTagFields.length - 1) {
        	ctx.font = "150px Arial";
        	ctx.fillText(val, 60, 250);

        	// Restore small font for safety (optional)
        	ctx.font = "18px Arial";
    	}
	});
	}
    }

    // Re-draw on form change
    [...oilFields, ...keyTagFields, 'labelStyle'].forEach(id => {
        if(document.getElementById(id)) {
            document.getElementById(id).addEventListener('input', drawLabel);
        }
    });

    // ------------------------
    // PRINT ZPL
    // ------------------------
    document.getElementById('printBtn').addEventListener('click', () => {
        if (!selectedDevice) { alert("No printer found"); return; }

        const template = document.getElementById('templateName').value.toUpperCase();
        let zpl = "";

        if(currentStyle === "style1") {
            const oilType = document.getElementById('oilType').value;
            const today = document.getElementById('today').value;
            const formattedMileage = document.getElementById('formattedMileage').value;
            const nextDate = document.getElementById('nextDate').value;

            zpl = `
^XA
^XF${template}^FS
^FN2^FD${oilType}^FS
^FN3^FD${today}^FS
${formattedMileage ? `^FN4^FD${formattedMileage}^FS` : ""}
${nextDate ? `^FN5^FD${nextDate}^FS` : ""}
^XZ
            `;
        } else {
    	const customer     = document.getElementById('customer').value;
    	const car          = document.getElementById('car').value;
    	const plate        = document.getElementById('plate').value;
    	const vin          = document.getElementById('vin').value;
    	const color        = document.getElementById('color').value;
    	const repairOrder  = document.getElementById('repairOrder').value;

    	zpl = `
^XA
^XF${template}^FS
^FN2^FD${customer}^FS
^FN3^FD${car}^FS
^FN4^FD${plate}^FS
^FN5^FD${vin}^FS
^FN6^FD${color}^FS
^FN7^FD${repairOrder}^FS
^FN8^FD${repairOrder}^FS
^XZ
    `;
}

        selectedDevice.send(zpl, undefined, err => {
            if(err) alert("Printer Error: " + err);
        });

        // Clear fields
        if(currentStyle === 'style1') oilFields.forEach(id => document.getElementById(id).value = '');
        else keyTagFields.forEach(id => document.getElementById(id).value = '');

        drawLabel();
    });

    initBrowserPrint();
    drawLabel();
});
// Clear Button
document.getElementById('clearBtn').addEventListener('click', () => {
    const allFields = [
        'oilType','mileage','today','formattedMileage','nextDate',
        'customer','car','plate','vin','color','repairOrder'
    ];

    allFields.forEach(id => {
        const el = document.getElementById(id);
        if(el) {
            el.value = '';
            
            // Trigger input event so drawLabel() fires properly
            const event = new Event('input', { bubbles: true });
            el.dispatchEvent(event);
        }
    });
});

