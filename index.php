<!DOCTYPE html>
<html lang="pl">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <meta http-equiv="X-UA-Compatible" content="ie=edge">
    <title>Sender</title>
</head>

<body>
    <?php
    $connection = mysqli_connect("localhost", "id15287372_root", "8#m?uJwXg){YGY4R", "id15287372_sender");
    if (isset($_POST["name"]) && isset($_POST["surname"])) {
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
    ?>
    Hello, World!
</body>

</html>