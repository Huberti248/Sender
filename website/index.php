<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css">
    <!-- Global site tag (gtag.js) - Google Analytics -->
    <script async src="https://www.googletagmanager.com/gtag/js?id=G-XE714HEX8C"></script>
    <script>
        window.dataLayer = window.dataLayer || [];

        function gtag() {
            dataLayer.push(arguments);
        }
        gtag('js', new Date());

        gtag('config', 'G-XE714HEX8C');
    </script>
    <title>Sender</title>
    <style>
        html,
        body {
            background-image: url("res/worldMap.jpg");
            background-size: cover;
            width: 100%;
            height: 100%;
            padding: 0;
            margin: 0;
        }

        #header {
            font-size: 48px;
        }

        #content {
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: flex-end;
            height: 85%;
        }

        #downloadBtn {
            background-color: DodgerBlue;
            border: none;
            color: white;
            cursor: pointer;
            font-size: 20px;
            border-radius: 15px;
            margin-bottom: 5px;
            padding: 15px;
            min-width: 20%;
            text-align: center;
            font-size: 30px;
        }

        #sourceCodeBtn {
            background-color: rgb(133, 208, 255);
            border-radius: 15px;
            padding: 15px;
            min-width: 20%;
            text-align: center;
        }

        #trailer {
            width: 100%;
            height: 100%;
            margin-bottom: 5px;
            opacity: 0.9;
        }

        @media (min-width: 768px) {
            #header {
                padding: 25px 10px 10px 80px;
            }

            #trailer {
                width: 60%;
            }
        }
    </style>
</head>

<body>
    <div id="header">
        Sender
    </div>
    <div id="content">
        <iframe id="trailer" src="https://youtube.com/embed/N7490Lpw6JU?autoplay=1&mute=1&playlist=N7490Lpw6JU&loop=1">
        </iframe>
        <div id="downloadBtn">Download soon</div>
        <a href="https://github.com/huberti1/Sender" id="sourceCodeBtn">
            <i class="fa fa-github" style="font-size:36px; text-align: center;"> Source code</i>
        </a>
    </div>
    <div style="display: flex; flex-direction: column; align-items: center;">
        <p style="font-size: 30px;">Join mailing list:</p>
        <?php
        if (isset($_POST['email'])) {
            if (filter_var($_POST['email'], FILTER_VALIDATE_EMAIL)) {
                // TODO: Prevent spam (capatche?)
                $connection = mysqli_connect("sql106.epizy.com", "epiz_28982080", "SS1ivC2Ywdf4c", "epiz_28982080_Sender");
                $stmt = mysqli_prepare($connection, "INSERT INTO `Users` (`id`, `email`) VALUES (NULL, ?)");
                $stmt->bind_param("s", $email);
                $email = $_POST['email'];
                $stmt->execute();
            } else {
                echo '<div style="color:red;">Invalid email. Try again</div>';
            }
        }
        ?>
        <form action="index.php" method="POST" style="display: flex;">
            <input placeholder="address@domain.com" name="email">
            <input type="submit" value="SUBSCRIBE">
        </form>
    </div>
</body>

</html>