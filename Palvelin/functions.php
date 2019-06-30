<?php
    function sqlConnect()
    {
        $sqlHost   = "mysql:host=localhost;dbname=mikrokontrollerit";
        $sqlUser   = "mysqluser";
        $sqlPass   = "12keke34";
        $sqlHandle = null;

        try
        {
            $sqlHandle = new PDO($sqlHost, $sqlUser, $sqlPass);
            $sqlHandle->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
        }
        catch(PDOException $e)
        {
            throw new Exception('SQL virhe: '.$e->getMessage());
        }

        return $sqlHandle;
    }

    function restClock()
    {
        print(date('H:i:sd.m.Y'));
    }

    function restUnlock()
    {
        try
        {
            $getKey = isset($_GET['key']) ? preg_replace('/[^0-9]/', '', $_GET['key']) : null;

            if (strlen($getKey) > 0)
            {
                $sqlHandle = sqlConnect();
                $sqlQuery = $sqlHandle->prepare("SELECT pin, message FROM pins WHERE pin = :pin");
                $sqlQuery->execute(array(':pin' => $getKey));
                $sqlRow = $sqlQuery->fetch(PDO::FETCH_ASSOC);

                if (is_array($sqlRow))
                {
                    print("1");

                    $sqlQuery = $sqlHandle->prepare("INSERT INTO logs (message) VALUES (:message)");
                    $sqlQuery->execute(array(':message' => "Lukko avattu avaimella ".$sqlRow['pin']." (".$sqlRow['message'].")"));
                }
                else
                {
                    print("0");

                    $sqlQuery = $sqlHandle->prepare("INSERT INTO logs (message) VALUES (:message)");
                    $sqlQuery->execute(array(':message' => "Lukkoa yritettiin avata avaimella ".$getKey));
                }
            }
            else
            {
                print("0");

                $sqlQuery = $sqlHandle->prepare("INSERT INTO logs (message) VALUES (:message)");
                $sqlQuery->execute(array(':message' => "Lukkoa yritettiin avata ilman avainkoodia"));
            }
        }
        catch (Exception $e)
        {
            print($e->getMessage());
        }
        finally
        {
            $sqlHandle = null;
        }
    }

    function restOverride()
    {
        try
        {
            $sqlHandle = sqlConnect();
            $sqlQuery = $sqlHandle->prepare("INSERT INTO logs (message) VALUES (:message)");
            $sqlQuery->execute(array(':message' => "Lukko avattu sisäpuolen napista"));
        }
        catch (Exception $e)
        {
            print($e->getMessage());
        }
        finally
        {
            $sqlHandle = null;
        }
    }

    function ajaxFetchLogs()
    {
        try
        {
            $sqlHandle = sqlConnect();
            $sqlQuery = $sqlHandle->prepare("SELECT id, ts, message FROM logs ORDER BY ts DESC LIMIT 100");
            $sqlQuery->execute();
            $sqlRows = $sqlQuery->fetchAll(PDO::FETCH_ASSOC);

            if (is_array($sqlRows) && count($sqlRows) > 0)
            {
                foreach ($sqlRows as &$row)
                {
                    $row['ts'] = date('d.m.Y H:i:s', strtotime($row['ts']));
                }

                print(json_encode($sqlRows));
            }
        }
        catch (Exception $e)
        {
            print($e->getMessage());
        }
        finally
        {
            $sqlHandle = null;
        }
    }

    function ajaxClearLogs()
    {
        try
        {
            $sqlHandle = sqlConnect();
            $sqlHandle->exec("DELETE FROM logs");
        }
        catch (Exception $e)
        {
            print($e->getMessage());
        }
        finally
        {
            $sqlHandle = null;
        }
    }

    function ajaxFetchPins()
    {
        try
        {
            $sqlHandle = sqlConnect();
            $sqlQuery = $sqlHandle->prepare("SELECT id, pin, message FROM pins");
            $sqlQuery->execute();
            $sqlRows = $sqlQuery->fetchAll(PDO::FETCH_ASSOC);

            if (is_array($sqlRows) && count($sqlRows) > 0)
            {
                print(json_encode($sqlRows));
            }
        }
        catch (Exception $e)
        {
            print($e->getMessage());
        }
        finally
        {
            $sqlHandle = null;
        }
    }

    function ajaxAddPin()
    {
        try
        {
            $getPin = isset($_GET['pin']) ? preg_replace('/[^0-9]/', '', $_GET['pin']) : null;
            $getMsg = isset($_GET['msg']) ? $_GET['msg'] : null;

            if (strlen($getPin) > 0)
            {
                $sqlHandle = sqlConnect();
                $sqlQuery = $sqlHandle->prepare("SELECT COUNT(id) FROM pins WHERE pin = :pin");
                $sqlQuery->execute(array(':pin' => $getPin));

                if ($sqlQuery->fetchColumn() == 0)
                {
                    $sqlQuery = $sqlHandle->prepare("INSERT INTO pins (pin, message) VALUES (:pin, :message)");
                    $sqlQuery->execute(array(':pin' => $getPin, ':message' => $getMsg));
                }
            }
        }
        catch (Exception $e)
        {
            print($e->getMessage());
        }
        finally
        {
            $sqlHandle = null;
        }
    }

    function ajaxDelPin()
    {
        try
        {
            $getID = isset($_GET['id']) ? preg_replace('/[^0-9]/', '', $_GET['id']) : null;

            if (strlen($getID) > 0)
            {
                $sqlHandle = sqlConnect();
                $sqlQuery = $sqlHandle->prepare("DELETE FROM pins WHERE id = :id");
                $sqlQuery->execute(array(':id' => $getID));
            }
        }
        catch (Exception $e)
        {
            print($e->getMessage());
        }
        finally
        {
            $sqlHandle = null;
        }
    }
?>
