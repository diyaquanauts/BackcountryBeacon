#ifndef FILES_H
#define FILES_H

const char files_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Files on SD Card</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 20px;
            background-color: #f4f4f4;
            color: #333;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: flex-start;
            min-height: 100vh;
            box-sizing: border-box;
        }
        h1 {
            margin: 0;
            padding: 20px;
            font-size: 32px;
            text-align: center;
        }
        input[type="text"] {
            width: 100%;
            max-width: 600px;
            padding: 10px;
            margin: 20px 0;
            font-size: 16px;
            border: 1px solid #ccc;
            border-radius: 5px;
            box-sizing: border-box;
        }
        .file-list {
            width: 100%;
            max-width: 600px;
            box-sizing: border-box;
            overflow-y: auto;
            max-height: calc(100vh - 200px); /* Adjust height based on other elements */
        }
        .file {
            padding: 15px;
            margin: 5px 0;
            background-color: #ffffff;
            box-sizing: border-box;
            word-wrap: break-word;
            border-radius: 8px;
            box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
            cursor: pointer;
            transition: background-color 0.3s ease;
        }
        .file:hover {
            background-color: #f0f0f0;
        }
        .file a {
            text-decoration: none;
            color: inherit;
            display: block;
            width: 100%;
            height: 100%;
        }
        .file:last-child {
            border-bottom: none;
        }
    </style>
    <script>
        function filterFiles() {
            var input, filter, fileDivs, a, i;
            input = document.getElementById('searchInput');
            filter = input.value.toUpperCase();
            fileDivs = document.getElementsByClassName('file');

            for (i = 0; i < fileDivs.length; i++) {
                a = fileDivs[i].getElementsByTagName('a')[0];
                if (a.innerHTML.toUpperCase().indexOf(filter) > -1) {
                    fileDivs[i].style.display = "";
                } else {
                    fileDivs[i].style.display = "none";
                }
            }
        }
    </script>
</head>
<body>
<h1>🏕️ FILES 🏔️</h1>
<input type="text" id="searchInput" onkeyup="filterFiles()" placeholder="Search for files...">
<div class="file-list">
    <!-- FILES_PLACEHOLDER -->
</div>
</body>
</html>
)rawliteral";

#endif // FILES_H
