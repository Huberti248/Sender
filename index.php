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
        $messages = mysqli_query($connection, "
        SELECT messages.topic,(SELECT persons.name FROM persons WHERE persons.id=messages.senderId) AS name,(SELECT persons.surname FROM persons WHERE persons.id=messages.senderId) AS surname,messages.date,messages.content
        FROM messages
        WHERE messages.receiverId=(
             SELECT persons.id
             FROM persons
              WHERE persons.name='Bar' AND persons.surname='Foo')");
        echo "\035";
        while ($row = $messages->fetch_assoc()) {
            echo $row['topic'], "\037", $row['name'], "\037", $row['surname'], "\037", $row['date'], "\037", $row['content'], "\036";
        }
        echo "\035";
    }
    ?>
    Hello, World!
</body>

</html>