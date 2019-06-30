"use strict";

window.addEventListener("load", function()
{
    window.page = new PageManager();
});

class PageManager
{
    constructor()
    {
        this.pinContainer = document.getElementById("pinContainer");
        this.logContainer = document.getElementById("logContainer");

        this.intervalTimer = null;
        this.intervalCount = null;
        this.intervalButton = document.getElementById("intervalButton");
        this.intervalVisual = document.getElementById("intervalVisual");

        this.newMsgField = document.getElementById("newMsgField");
        this.newPinField = document.getElementById("newPinField");

        this.fetchLogs();
        this.fetchPins();
        this.resumeInterval();
    }

    pauseInterval()
    {
        clearInterval(this.intervalTimer);
        this.intervalButton.setAttribute("onclick", "page.resumeInterval();");
        this.intervalButton.value = "Jatka";
    }

    resumeInterval()
    {
        this.intervalCount = 0;
        this.intervalTimer = setInterval(function() {this.triggerInterval()}.bind(this), 10);
        this.intervalButton.setAttribute("onclick", "page.pauseInterval();");
        this.intervalButton.value = "Pysäytä";
    }

    triggerInterval()
    {
        this.intervalCount++;
        this.intervalVisual.style.background = "linear-gradient(to right, #d0d9e8 " + this.intervalCount + "%, #ffffff " + this.intervalCount + "%)";

        if (this.intervalCount >= 100)
        {
            this.intervalCount = 0;
            this.fetchLogs();
        }
    }

    validateNewPin()
    {
        this.newPinField.value = this.newPinField.value.replace(/[^0-9]/g, "");
    }

    submitNewPin(e)
    {
        if (e.keyCode == 13)
        {
            this.addPin();
        }
    }

    fetchLogs()
    {
        this.sendRequest("ajax.php?action=fetchLogs", this.printlogs.bind(this));
    }

    clearLogs()
    {
        this.sendRequest("ajax.php?action=clearLogs", this.printlogs.bind(this));
    }

    fetchPins()
    {
        this.sendRequest("ajax.php?action=fetchPins", this.printPins.bind(this));
    }

    addPin()
    {
        var newPin = this.newPinField.value.replace(/[^0-9]/g, "");
        var newMsg = this.newMsgField.value;

        if (newMsg.length < 1 || newPin.length < 1)
        {
            alert("Anna sekä kuvaus että avainkoodi");
        }
        else
        {
            var values  = "&pin=" + newPin;
                values += "&msg=" + newMsg;

            this.newPinField.value = "";
            this.newMsgField.value = "";

            this.sendRequest("ajax.php?action=addPin" + values, this.fetchPins.bind(this));
        }
    }

    removePin(pinID)
    {
        var values = "&id=" + pinID.toString().replace(/[^0-9]/g, "");

        if (values.length > 4)
        {
            console.log("asd?");
            this.sendRequest("ajax.php?action=delPin" + values, this.fetchPins.bind(this));
        }
    }

    sendRequest(request, callback)
    {
        try
        {
            var ajaxRequest = new XMLHttpRequest();

            ajaxRequest.open('GET', request, true);
            ajaxRequest.send(null);

            ajaxRequest.onreadystatechange = function()
            {
                if (ajaxRequest.readyState === 4)
                {
                    if (ajaxRequest.status === 200)
                    {
                        callback(ajaxRequest.responseText);
                    }
                }
                else
                {
                    console.log("Ajax status: " + ajaxRequest.status);
                }
            };
        }
        catch(err)
        {
            console.log("Ajax error: " + err);
        }
    }

    printPins(ajaxResponse)
    {
        var output = "";

        try
        {
            if (ajaxResponse.length < 1)
            {
                output = '<span class="wide">Kannassa ei ole avaimia</span>';
            }
            else
            {
                var ajaxArray = JSON.parse(ajaxResponse);
                var rowCount  = ajaxArray.length;
                var rowClass  = "";

                for (var i = 0; i < rowCount; i++)
                {
                    rowClass = i % 2 == 0 ? 'bgcG' : 'bgcW';
                    output += '<span class="' + rowClass + '">' + ajaxArray[i].message + '</span>';
                    output += '<span class="' + rowClass + '">' + ajaxArray[i].pin + '</span>';
                    output += '<span class="' + rowClass + '"><input type="button" value="Poista" onclick="page.removePin(' + ajaxArray[i].id + ');" /></span>';
                }
            }
        }
        catch(err)
        {
            output = '<span class="wide">JSON error: ' + err + '</span>';
        }

        // this.pinContainer.innerHTML = "<table>" + output + "</table>";
        this.pinContainer.innerHTML = output;
        this.pinContainer.scroll(0,0);
    }

    printlogs(ajaxResponse)
    {
        var output = "";

        try
        {
            if (ajaxResponse.length < 1)
            {
                output = '<span class="wide">Kannassa ei ole tapahtumia</span>';
            }
            else
            {
                var ajaxArray = JSON.parse(ajaxResponse);
                var rowCount  = ajaxArray.length;
                var rowClass  = "";

                for (var i = 0; i < rowCount; i++)
                {
                    rowClass = i % 2 == 0 ? 'bgcG' : 'bgcW';
                    output += '<span class="' + rowClass + '">' + ajaxArray[i].ts + '</span>';
                    output += '<span class="' + rowClass + '">' + ajaxArray[i].message + '</span>';
                }
            }
        }
        catch(err)
        {
            output = '<span class="wide">JSON error: ' + err + '</span>';
        }

        this.logContainer.innerHTML = output;
        this.logContainer.scroll(0,0);
    }
}
