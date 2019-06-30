<?php
    header("Content-Type: text/plain");
    header("Access-Control-Allow-Origin: *");

    if (isset($_GET['action']))
    {
        require_once('functions.php');

        switch (strtolower($_GET['action']))
        {
            case "fetchlogs":
                ajaxFetchLogs();
                break;

            case "clearlogs":
                ajaxClearLogs();
                break;

            case "fetchpins":
                ajaxFetchPins();
                break;

            case "addpin":
                ajaxAddPin();
                break;

            case "delpin":
                ajaxDelPin();
                break;
        }
    }
?>
