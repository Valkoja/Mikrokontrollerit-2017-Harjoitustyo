<?php
    header("Content-Type: text/plain");
    date_default_timezone_set('Europe/Helsinki');

    if (isset($_GET['action']))
    {
        require_once('functions.php');

        switch (strtolower($_GET['action']))
        {
            case "clock":
                restClock();
                break;

            case "unlock":
                restUnlock();
                break;

            case "override":
                restOverride();
                break;
        }
    }
?>
