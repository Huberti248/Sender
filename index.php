<!DOCTYPE html>
<html lang="pl">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <meta http-equiv="X-UA-Compatible" content="ie=edge">
    <title>Sender</title>
    <style>
        html,
        body {
            margin: 0;
            padding: 0;
            width: 100%;
            height: 100%;
            background-color: black;
        }

        #header {
            display: flex;
            justify-content: center;
            align-items: center;
            padding-top: 50px;
        }

        #btn {
            display: flex;
            justify-content: center;
            align-items: center;
            background-color: rgb(50, 125, 168);
            color: white;
            width: 30%;
            height: 50px;
            border-radius: 20px;
            cursor: pointer;
            font-size: 20px;
            font-family: Arial, Helvetica, sans-serif;
            text-decoration: none;
        }
    </style>
</head>

<body>
    <?php
    $connection = mysqli_connect("localhost", "id15287372_root", "8#m?uJwXg){YGY4R", "id15287372_sender");
    if (!isset($_POST["operation"]) && isset($_POST["name"]) && isset($_POST["surname"])) {
        $result = mysqli_query($connection, "SELECT * FROM persons WHERE persons.name='{$_POST['name']}' AND persons.surname='{$_POST['surname']}'");
        $rows = mysqli_num_rows($result);
        if ($rows == 0) {
            mysqli_query($connection, "INSERT INTO `persons` (`id`, `name`, `surname`) VALUES (NULL, '{$_POST['name']}', '{$_POST['surname']}');");
        }
        $messages = mysqli_query($connection, "SELECT messages.topic,(SELECT persons.name FROM persons WHERE persons.id=messages.senderId) AS name,(SELECT persons.surname FROM persons WHERE persons.id=messages.senderId) AS surname,messages.dateTime,messages.content
        FROM messages
        WHERE messages.receiverId=(
             SELECT persons.id
             FROM persons
              WHERE persons.name='{$_POST['name']}' AND persons.surname='{$_POST['surname']}')");
        echo "\035";
        while ($row = $messages->fetch_assoc()) {
            echo $row['topic'], "\037", $row['name'], "\037", $row['surname'], "\037", $row['dateTime'], "\037", $row['content'], "\036";
        }
        echo "\035";
    }
    if (isset($_POST["operation"]) && isset($_POST["name"]) && isset($_POST["surname"])) {
        if ($_POST["operation"] == "checkCalls") {
            $result = mysqli_query($connection, "SELECT calls.state AS state, (SELECT persons.name FROM persons WHERE persons.id=calls.callerId) AS name,
            (SELECT persons.surname FROM persons WHERE persons.id=calls.callerId) AS surname FROM calls WHERE calls.receiverId=(SELECT persons.id FROM persons WHERE persons.name='{$_POST["name"]}' AND persons.surname='{$_POST["surname"]}')");
            echo "\035";
            while ($row = $result->fetch_assoc()) {
                echo $row['state'], "\037", $row['name'], "\037", $row['surname'], "\036";
            }
            echo "\035";
        } else if ($_POST["operation"] == "acceptCall") {
            mysqli_query($connection, "UPDATE calls 
            SET state='Going' 
            WHERE calls.receiverId=(SELECT persons.id FROM persons WHERE persons.name='{$_POST["name"]}' AND persons.surname='{$_POST["surname"]}')");
        }
    }
    if (
        isset($_POST["receiverName"]) &&
        isset($_POST["receiverSurname"]) &&
        isset($_POST["topic"]) &&
        isset($_POST["content"]) &&
        isset($_POST["senderName"]) &&
        isset($_POST["senderSurname"])
    ) {
        // TODO: In NOW() function the value is expressed in the session time zone. What does it mean and is it going to be 1 hour to early in Poland? Is it connected with Windows region settings?
        mysqli_query($connection, "INSERT INTO `messages` (`id`, `receiverId`, `senderId`, `dateTime`, `topic`, `content`) 
        VALUES (NULL, (SELECT persons.id 
                       FROM persons 
                       WHERE persons.name='{$_POST['receiverName']}' AND persons.surname='{$_POST['receiverSurname']}'), (SELECT persons.id 
                       FROM persons 
                       WHERE persons.name='{$_POST['senderName']}' AND persons.surname='{$_POST['senderSurname']}'), NOW(), '{$_POST['topic']}', '{$_POST['content']}');
        ");
    }
    if (isset($_POST["callerName"]) && isset($_POST["callerSurname"]) && isset($_POST["receiverName"]) && isset($_POST["receiverSurname"])) {
        mysqli_query($connection, "INSERT INTO calls
        (calls.callerId,calls.receiverId,calls.state)
        VALUES (
            (SELECT persons.id FROM persons WHERE persons.name='{$_POST["callerName"]}' AND persons.surname='{$_POST["callerSurname"]}'),
            (SELECT persons.id FROM persons WHERE persons.name='{$_POST["receiverName"]}' AND persons.surname='{$_POST["receiverSurname"]}'),
            'Pending')");
    }
    if (isset($_POST["callerName"]) && isset($_POST["callerSurname"]) && isset($_POST["samples"]) && isset($_POST["sampleCount"]) && isset($_POST["channelCount"]) && isset($_POST["sampleRate"])) {
        mysqli_query($connection, "UPDATE `calls` 
        SET `callerAudioData`=CONCAT((SELECT calls.callerAudioData FROM calls WHERE callerId=(SELECT persons.id FROM persons WHERE persons.name='{$_POST["callerName"]}' AND persons.surname='{$_POST["callerSurname"]}')),{$_POST["callerAudioData"]})
        WHERE callerId=(SELECT persons.id FROM persons WHERE persons.name='{$_POST["callerName"]}' AND persons.surname='{$_POST["callerSurname"]}')");
    }
    if (isset($_POST["receiverName"]) && isset($_POST["receiverSurname"])) {
        $result = mysqli_query($connection, "SELECT calls.callerAudioData AS callerAudioData
        FROM calls
        WHERE calls.receiverId=(SELECT persons.id FROM persons WHERE persons.name='{$_POST['receiverName']}' AND persons.surname='{$_POST['receiverSurname']}')");
        $row = mysqli_fetch_assoc($result);
        echo "\035";
        echo $row["callerAudioData"];
        echo "\035";
    }
    ?>
    <div id="header">
        <a href="https://drive.google.com/u/0/uc?id=1cg9wjPeDTq_y2JAiVS3NCDYjxPKwrKfd&export=download" id="btn">
            <div>Pobierz</div>
        </a>
    </div>
</body>

</html>