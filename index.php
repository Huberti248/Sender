<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Sender</title>
    <style>
        html,
        body {
            margin: 0;
            padding: 0;
            width: 100%;
            height: 100%;
            background-color: black;
            color: white;
        }

        #login {
            width: 100%;
            height: 100%;
            display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
        }

        input {
            width: 35%;
            height: 25px;
        }

        #button {
            background-color: rgb(35, 134, 54);
            width: 15%;
            height: 30px;
            display: flex;
            justify-content: center;
            align-items: center;
            cursor: pointer;
            color: white;
        }
    </style>
</head>

<body>
    <?php
    function showLogin()
    {
        echo '<form id="login" method="POST" action="index.php">';
        echo '<input name="email" style="margin-bottom: 15px;" placeholder="Email">';
        echo '<input name="password" style="margin-bottom: 15px;" placeholder="Password" type="password">';
        echo '<button id="button" type="submit">Zaloguj</button>';
        echo '</form>';
    }

    $connection = mysqli_connect("localhost", "root", "", "sender");
    if (isset($_POST['email']) && isset($_POST['password'])) {
        $result = mysqli_query($connection, "SELECT * 
        FROM users 
        WHERE users.email='{$_POST['email']}' && users.password='{$_POST['password']}'");
        if (mysqli_num_rows($result) > 0) {
            //TODO Display messages.
        } else {
            showLogin();
        }
    } else {
        showLogin();
    }
    ?>
</body>

</html>