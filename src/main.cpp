#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <ESPmDNS.h>
#else
#error "Board not found"
#endif
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <Ticker.h>

int flexPin1 = 36;
int flexPin2 = 39;
int flexPin3 = 34;
int flexPin4 = 35;
int flexPin5 = 33;

Ticker timer;

char webpage[] PROGMEM = R"=====(

<!DOCTYPE html>
<html lang="en">
<head>
     <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Sign Wave</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: #f0f0f5;
            margin: 0;
            padding: 0;
        }
        .tab {
            display: none;
        }
        .tab-content {
            margin-top: 20px;
            padding: 20px;
            background-color: #fff;
            border: 1px solid #ddd;
            border-radius: 8px;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
        }
        .tabs {
            overflow: hidden;
            background: #333;
            border-radius: 8px 8px 0 0;
            margin: 20px;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
        }
        .tabs button {
            background: #444;
            color: white;
            float: left;
            border: none;
            outline: none;
            cursor: pointer;
            padding: 14px 16px;
            transition: background 0.3s;
        }
        .tabs button:hover {
            background: #555;
        }
        .tabs button.active {
            background: #666;
        }
        .sound-button {
            background-color: #4CAF50;
            color: white;
            padding: 12px 20px;
            border: none;
            cursor: pointer;
            margin-top: 10px;
            border-radius: 5px;
            transition: background-color 0.3s;
        }
        .sound-button:hover {
            background-color: #45a049;
        }
        h2, h3 {
            color: #333;
        }
        meter {
            width: 100%;
            height: 20px;
        }
        .flex-container {
            display: flex;
            justify-content: space-evenly; /* Evenly space items */
            flex-wrap: wrap; /* Allow items to wrap */
            margin-top: 30px;
    
        }
        .flex-item {
            text-align: center;
            flex: 1 1 200px; /* Flex properties */
            margin: 10px;
            padding: 20px;
            background-color: #fff;
            border: 1px solid #ddd;
            border-radius: 8px;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
        }

        @media (max-width: 375px) {  
          .flex-container {
            flex-direction: column; 
          }
        }

        .flex-item h3 {
            margin: 10px 0;
        }

        .prediction {
          margin-top: 30px; 
        }

        
        .value {
            font-size: 24px;
            color: #777;
        }
    </style>
</head>
<body>

<div class="tabs">
    <button class="tab-link" onclick="openTab(event, 'Learn')">Translate</button>
</div>

<center>
<div id="Learn" class="tab active">
    <h2>Translate</h2>
    <p>This is the Translate tab. This feature translates SignWave's data into a speech output. This feature can be accessed by numeous devices at the same time.</p>
    <button class="sound-button" id="speakButton" onclick="speakContent()">Text to Speech</button
</div>

<div id="prediction">
    <h2 id = "letterlabel"> Predicted Letter: </h2>
    <h1 id="myElement">  </h1>
</div>


<div class="flex-container">
  <div class="flex_item">
    <h3>Thumb</h3>
    <meter value="2" min="0" max="100" id="flex2_meter"> </meter>
    <h3 id="flex2_value" style="display: inline-block;"> 2 </h3>
  </div>
  
  <div class="flex_item">
    <h3>Index Finger</h3>
    <meter value="2" min="0" max="100" id="flex1_meter"> </meter>
    <h3 id="flex1_value" style="display: inline-block;"> 2 </h3>
  </div>

  <div class="flex_item">
    <h3>Middle Finger</h3>
    <meter value="2" min="0" max="100" id="flex3_meter"> </meter>
    <h3 id="flex3_value" style="display: inline-block;"> 2 </h3>
  </div>
   
   <div class="flex_item">
    <h3>Ring Finger</h3>
    <meter value="2" min="0" max="100" id="flex5_meter"> </meter>
    <h3 id="flex5_value" style="display: inline-block;"> 2 </h3>
   </div>

   <div class="flex_item">
    <h3>Pinky</h3>
    <meter value="2" min="0" max="100" id="flex4_meter"> </meter>
    <h3 id="flex4_value" style="display: inline-block;"> 2 </h3>
  </div>

</div>
  

</center>

<script>
    function openTab(evt, tabName) {
        var i, tabcontent, tablinks;
        tabcontent = document.getElementsByClassName("tab-content");
        for (i = 0; i < tabcontent.length; i++) {
            tabcontent[i].style.display = "none";
        }
        tablinks = document.getElementsByClassName("tab-link");
        for (i = 0; i < tablinks.length; i++) {
            tablinks[i].className = tablinks[i].className.replace(" active", "");
        }
        document.getElementById(tabName).style.display = "block";
        evt.currentTarget.className += " active";
    }

    const myElement = document.getElementById("myElement");
    const speaker = new SpeechSynthesisUtterance();

    function speakContent() {
      const content = myElement.textContent;
      if (content.trim() !== "") {
        speaker.text = content;
        window.speechSynthesis.speak(speaker);
      }
    }
    
    speakContent();

    const observer = new MutationObserver(speakContent);
    observer.observe(myElement, { childList: true });

    // Open the default tab
    document.getElementsByClassName('tab-link')[0].click();

    var connection = new WebSocket('ws://'+location.hostname+':81/');
    var flex1_data = 0; 
    var flex2_data = 0;
    var flex3_data = 0;
    var flex4_data = 0;
    var flex5_data = 0;

    connection.onmessage = function(event){
       var full_data = event.data;
        console.log(full_data);

        var data = JSON.parse(full_data);
        flex1_data = data.flex1;   
        flex2_data = data.flex2;
        flex3_data = data.flex3;
        flex4_data = data.flex4;
        flex5_data = data.flex5;

        var index = flex1_data * 40.95
        var thumb = flex2_data * 40.95
        var middle = flex3_data * 40.95
        var pinky = flex4_data * 40.95
        var ring = flex5_data * 40.95

        document.getElementById("flex1_meter").value = flex1_data;
        document.getElementById("flex1_value").innerHTML = Math.round(index);
        document.getElementById("flex2_meter").value = flex2_data;
        document.getElementById("flex2_value").innerHTML = Math.round(thumb); 
        document.getElementById("flex3_meter").value = flex3_data;
        document.getElementById("flex3_value").innerHTML = Math.round(middle);
        document.getElementById("flex4_meter").value = flex4_data;
        document.getElementById("flex4_value").innerHTML = Math.round(pinky);
        document.getElementById("flex5_meter").value = flex5_data;
        document.getElementById("flex5_value").innerHTML = Math.round(ring);

        
        if (index < 500 && middle < 500 && ring < 2500 && pinky < 700 && thumb > 2500) {
          document.getElementById("myElement").textContent = 'a';
         } 
        else if (index < 900 && middle < 300 && ring < 1500 && pinky < 1000 && thumb < 1000 && thumb > 300) {
          document.getElementById("myElement").textContent = 'e';
         } 
        else if (thumb > 3000 && index < 3000 && middle < 300 && ring < 3000 && pinky < 1000 && index > 1000 && ring > 1000){
          document.getElementById("myElement").textContent = 'o';
        }
        else if (index > 3500 && thumb > 1500 && middle < 100 && ring < 1500 && pinky < 1000 && thumb < 3000) {
          document.getElementById("myElement").textContent = 'd';
        }
        else if (thumb < 2500 && index > 4000 && middle > 3000 && ring > 800 & pinky > 2000) {
          document.getElementById("myElement").textContent = 'b';
        }
        else if (thumb > 3000 && index > 2000 && index < 4095 && middle < 1000 && ring < 4000 && pinky > 1000) {
          document.getElementById("myElement").textContent = 'c';
        }
        else if (thumb < 4000 && index < 2500 && middle > 3000 && ring > 3000 && pinky > 3000){
          document.getElementById("myElement").textContent = 'f';
        }
        else if (thumb > 3500 && index > 3000 && middle < 100 && ring < 1000 && pinky < 500){
          document.getElementById("myElement").textContent = 'g';
        }
        else if (thumb < 2200 && index > 3000 && middle > 3000 && ring < 1800 && pinky < 1000 && ring > 600){
          document.getElementById("myElement").textContent = 'h';
        }
        else if (thumb < 1000 && index < 800 && middle < 500 && ring < 1700 && pinky > 3000){
          document.getElementById("myElement").textContent = 'i';
        }
        else if (thumb > 3000 && index > 3000 && middle > 3000 && ring < 3000 && pinky < 1000){
          document.getElementById("myElement").textContent = 'k';
        }
        else if (thumb > 2000 && index > 3000 && middle < 500 && ring < 1000 && pinky < 500){
          document.getElementById("myElement").textContent = 'l';
        }
        else if (thumb < 400 && index < 800 && middle < 400 && ring < 1000 && pinky < 400){
          document.getElementById("myElement").textContent = 'm';
        }
        else if (thumb < 100 && index < 800 && middle < 100 && ring < 1200 && pinky < 400 && ring > 500){
          document.getElementById("myElement").textContent = 'm';
        }
        else if (thumb < 300 && index < 3000 && middle < 300 && ring < 1800 && pinky < 600 && index > 1500){
          document.getElementById("myElement").textContent = 'n';
        }
        else if (thumb > 3000 && index > 3000 && middle > 500 && ring < 1500 && pinky < 600 && middle < 1500){
          document.getElementById("myElement").textContent = 'p';
        }
        else if (thumb > 3500 && index > 2000 && middle < 100 && ring < 1000 && pinky < 500 && index < 4000){
          document.getElementById("myElement").textContent = 'q';
        }
         else if (thumb < 4000 && index < 1500 && middle > 3000 && ring < 2500 && pinky < 900){
          document.getElementById("myElement").textContent = 'r';
        }
         else if (thumb < 3000 && index < 500 && middle < 100 && ring < 1000 && pinky < 300 && thumb > 1000){
          document.getElementById("myElement").textContent = 's';
        }
        else if (thumb < 1000 && index < 3000 && middle < 300 && ring < 1500 && pinky < 300 && index > 1000){
          document.getElementById("myElement").textContent = 't';
        }
        else if (thumb < 3000 && index > 3000 && middle > 3000 && ring < 600 && pinky < 600){
          document.getElementById("myElement").textContent = 'u';
        }
        else if (thumb < 2500 && index > 3000 && middle < 3000 && ring < 1500 && pinky < 600 && middle > 2000){
          document.getElementById("myElement").textContent = 'v';
        }
        else if (thumb < 2500 && index > 3000 && middle > 3000 && ring > 3000 && pinky < 600){
          document.getElementById("myElement").textContent = 'w';
        }
         else if (thumb < 2000 && index > 1500 && middle < 100 && ring < 1000 && pinky < 600 && index < 4095){
          document.getElementById("myElement").textContent = 'x';
        }
         else if (thumb > 3000 && index < 2500 && middle < 1000 && ring < 1000 && pinky > 3000){
          document.getElementById("myElement").textContent = 'y';
        }


        else {
         document.getElementById("myElement").textContent = '';
         }
      

       }

    

    
</script>

</body>
</html>

)=====";

char learnpage[] PROGMEM = R"=====(

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Sign Wave</title>
    <style>
          body {
            font-family: Arial, sans-serif;
            background-color: #f0f0f5;
            margin: 0;
            padding: 0;
        }
        .tab {
            display: none;
        }
        .tab-content {
            margin-top: 20px;
            padding: 20px;
            background-color: #fff;
            border: 1px solid #ddd;
            border-radius: 8px;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
        }
        .tabs {
            overflow: hidden;
            background: #333;
            border-radius: 8px 8px 0 0;
            margin: 20px;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
        }
        .tabs button {
            background: #444;
            color: white;
            float: left;
            border: none;
            outline: none;
            cursor: pointer;
            padding: 14px 16px;
            transition: background 0.3s;
        }
        .tabs button:hover {
            background: #555;
        }
        .tabs button.active {
            background: #666;
        }
        .sound-button {
            background-color: #4CAF50;
            color: white;
            padding: 12px 20px;
            border: none;
            cursor: pointer;
            margin-top: 10px;
            border-radius: 5px;
            transition: background-color 0.3s;
        }
        .sound-button:hover {
            background-color: #45a049;
        }
        h2, h3 {
            color: #333;
        }
        meter {
            width: 100%;
            height: 20px;
        }
        .flex-section {
            text-align: center;
            margin: 20px 0;
            padding: 20px;
            background-color: #fff;
            border: 1px solid #ddd;
            border-radius: 8px;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
        }
        .flex-section h3 {
            margin: 10px 0;
        }
    </style>
</head>
<body>

<div class="tabs">
    <button class="tab-link" onclick="openTab(event, 'Learn')">Learn</button>
</div>

<div id="Learn" class="tab tab-content">
    <h2>Learn</h2>

    <div id="quiz">
        <h3>Gesture Quiz</h3>
        <p id="quizLetter"></p>
        <button onclick="startQuiz()" id = "quizButton">Start Quiz</button>
        <p id="quizResult"></p>
        <p id="thumb_fix"> </p>
        <p id="index_fix"> </p>
        <p id= "middle_fix"> </p>
        <p id= "ring_fix"> </p>
        <p id= "pinky_fix"> </p>
    </div>
</div>

<center>
<div id="prediction">
    <h2 id = "letterlabel"> Predicted Letter: </h2>
    <h1 id="myElement">  </h1>
</div>
</center>

<div class="flex-section">
    <h3>Thumb</h3>
    <meter value="2" min="0" max="100" id="flex2_meter"> </meter>
    <h3 id="flex2_value" style="display: inline-block;"> 2 </h3>
  
    <h3>Index Finger</h3>
    <meter value="2" min="0" max="100" id="flex1_meter"> </meter>
    <h3 id="flex1_value" style="display: inline-block;"> 2 </h3>

    <h3>Middle Finger</h3>
    <meter value="2" min="0" max="100" id="flex3_meter"> </meter>
    <h3 id="flex3_value" style="display: inline-block;"> 2 </h3>

    <h3>Ring Finger</h3>
    <meter value="2" min="0" max="100" id="flex5_meter"> </meter>
    <h3 id="flex5_value" style="display: inline-block;"> 2 </h3>

    <h3>Pinky</h3>
    <meter value="2" min="0" max="100" id="flex4_meter"> </meter>
    <h3 id="flex4_value" style="display: inline-block;"> 2 </h3>

</div>

<script>
    function openTab(evt, tabName) {
        var i, tabcontent, tablinks;
        tabcontent = document.getElementsByClassName("tab-content");
        for (i = 0; i < tabcontent.length; i++) {
            tabcontent[i].style.display = "none";
        }
        tablinks = document.getElementsByClassName("tab-link");
        for (i = 0; i < tablinks.length; i++) {
            tablinks[i].className = tablinks[i].className.replace(" active", "");
        }
        document.getElementById(tabName).style.display = "block";
        evt.currentTarget.className += " active";
    }

    function checkNumber() {
        var number = document.getElementById("numberInput").value;
        var output = document.getElementById("output");
        if (number < 1000) {
            output.textContent = "9";
            document.getElementById("speakButton").style.display = "block";
        } else {
            output.textContent = "";
            document.getElementById("speakButton").style.display = "none";
        }
    }

    function speakNumber() {
        var outputText = document.getElementById("output").textContent;
        var utterance = new SpeechSynthesisUtterance(outputText);
        window.speechSynthesis.speak(utterance);
    }

    // Open the default tab
    document.getElementsByClassName('tab-link')[0].click();

    // Quiz Logic
    var letters = ['A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y'];
    var currentLetter = '';
    var letterRanges = {
        'A': { thumb: [2001, 5000], index: [0, 499], middle: [0, 499], ring: [0, 2499], pinky: [0, 699] },
        'B': { thumb: [0, 2499], index: [4001, 5000], middle: [3001, 5000], ring: [801, 5000], pinky: [2001, 5000] },
        'C': { thumb: [3001, 5000], index: [2001, 4094], middle: [0, 999], ring: [0, 3999], pinky: [1001, 5000] },
        'D': { thumb: [1501, 2999], index: [3501, 5000], middle: [0, 99], ring: [0, 1499], pinky: [0, 999] },
        'E': { thumb: [299, 999], index: [0, 899], middle: [0, 299], ring: [0, 1499], pinky: [0, 999]},
        'F': { thumb: [0, 3999], index: [0, 2499], middle: [3001, 5000], ring: [3001, 5000], pinky: [3001, 5000]},
        'G': { thumb: [3501, 5000], index: [3001, 5000], middle: [0, 99], ring: [0, 999], pinky: [0, 499]},
        'H': { thumb: [0, 2199], index: [3001, 5000], middle: [3001, 5000], ring: [601, 1799], pinky: [0, 999]},
        'I': { thumb: [0, 999], index: [0, 799], middle: [0, 499], ring: [0, 1699], pinky: [3001, 5000]},
        'K': { thumb: [3001, 5000], index: [3001, 5000], middle: [3001, 5000], ring: [0, 2999], pinky: [0, 999]},
        'L': { thumb: [2001, 5000], index: [3001, 5000], middle: [0, 499], ring: [0, 999], pinky: [0, 499]},
        'M': { thumb: [0, 99], index: [0, 799], middle: [0, 99], ring: [501, 1199], pinky: [0, 399]},
        'N': { thumb: [0, 299], index: [1501, 2999], middle: [0, 299], ring: [0, 1799], pinky: [0, 599]},
        'O': { thumb: [3001, 5000], index: [1001, 2999], middle: [0, 299], ring: [1001, 2999], pinky: [0, 999]},
        'P': { thumb: [3001, 5000], index: [3001, 5000], middle: [501, 1499], ring: [0, 1499], pinky: [0, 599]},
        'Q': { thumb: [3501, 5000], index: [2001, 3999], middle: [0, 99], ring: [0, 999], pinky: [0, 499]},
        'R': { thumb: [0, 3999], index: [0, 1499], middle: [3001, 5000], ring: [0, 2499], pinky: [0, 899]},
        'S': { thumb: [1001, 2999], index: [0, 499], middle: [0, 99], ring: [0, 999], pinky: [0, 299]},
        'T': { thumb: [0, 999], index: [1001, 2999], middle: [0, 299], ring: [0, 1499], pinky: [0, 299]},
        'U': { thumb: [0, 2999], index: [3001, 5000], middle: [3001, 5000], ring: [0, 599], pinky: [0, 599]},
        'V': { thumb: [0, 2499], index: [3001, 5000], middle: [2001, 2999], ring: [0, 1499], pinky: [0, 599]},
        'W': { thumb: [0, 2499], index: [3001, 5000], middle: [3001, 5000], ring: [3001, 5000], pinky: [0, 599]},
        'X': { thumb: [0, 1999], index: [1501, 4094], middle: [0, 99], ring: [0, 999], pinky: [0, 599]},
        'Y': { thumb: [3001, 5000], index: [0, 2499], middle: [0, 999], ring: [0, 999], pinky: [3001, 5000]},
        
        // Add ranges for other letters...
    };

    function startQuiz() {
        currentLetter = letters[Math.floor(Math.random() * letters.length)];
        document.getElementById('quizLetter').textContent = 'Show the gesture for letter: ' + currentLetter;
        document.getElementById('quizResult').textContent = '';
        document.getElementById('quizButton').textContent = 'Next Sign';
    }

    var connection = new WebSocket('ws://'+location.hostname+':81/');
    var flex1_data = 0; 
    var flex2_data = 0;
    var flex3_data = 0;
    var flex4_data = 0;
    var flex5_data = 0;

    connection.onmessage = function(event){
        var full_data = event.data;
        console.log(full_data);

        var data = JSON.parse(full_data);
        flex1_data = data.flex1;   
        flex2_data = data.flex2;
        flex3_data = data.flex3;
        flex4_data = data.flex4;
        flex5_data = data.flex5;

        var index = flex1_data * 40.95;
        var thumb = flex2_data * 40.95;
        var middle = flex3_data * 40.95;
        var pinky = flex4_data * 40.95;
        var ring = flex5_data * 40.95;

        document.getElementById("flex1_meter").value = flex1_data;
        document.getElementById("flex1_value").innerHTML = index;
        document.getElementById("flex2_meter").value = flex2_data;
        document.getElementById("flex2_value").innerHTML = thumb; 
        document.getElementById("flex3_meter").value = flex3_data;
        document.getElementById("flex3_value").innerHTML = middle;
        document.getElementById("flex4_meter").value = flex4_data;
        document.getElementById("flex4_value").innerHTML = pinky;
        document.getElementById("flex5_meter").value = flex5_data;
        document.getElementById("flex5_value").innerHTML = ring;

        checkGesture(index, thumb, middle, ring, pinky);

       if (index < 500 && middle < 500 && ring < 2500 && pinky < 700 && thumb > 2500) {
          document.getElementById("myElement").textContent = 'a';
         } 
        else if (index < 900 && middle < 300 && ring < 1500 && pinky < 1000 && thumb < 1000 && thumb > 300) {
          document.getElementById("myElement").textContent = 'e';
         } 
        else if (thumb > 3000 && index < 3000 && middle < 300 && ring < 3000 && pinky < 1000 && index > 1000 && ring > 1000){
          document.getElementById("myElement").textContent = 'o';
        }
        else if (index > 3500 && thumb > 1500 && middle < 100 && ring < 1500 && pinky < 1000 && thumb < 3000) {
          document.getElementById("myElement").textContent = 'd';
        }
        else if (thumb < 2500 && index > 4000 && middle > 3000 && ring > 800 & pinky > 2000) {
          document.getElementById("myElement").textContent = 'b';
        }
        else if (thumb > 3000 && index > 2000 && index < 4095 && middle < 1000 && ring < 4000 && pinky > 1000) {
          document.getElementById("myElement").textContent = 'c';
        }
        else if (thumb < 4000 && index < 2500 && middle > 3000 && ring > 3000 && pinky > 3000){
          document.getElementById("myElement").textContent = 'f';
        }
        else if (thumb > 3500 && index > 3000 && middle < 100 && ring < 1000 && pinky < 500){
          document.getElementById("myElement").textContent = 'g';
        }
        else if (thumb < 2200 && index > 3000 && middle > 3000 && ring < 1800 && pinky < 1000 && ring > 600){
          document.getElementById("myElement").textContent = 'h';
        }
        else if (thumb < 1000 && index < 800 && middle < 500 && ring < 1700 && pinky > 3000){
          document.getElementById("myElement").textContent = 'i';
        }
        else if (thumb > 3000 && index > 3000 && middle > 3000 && ring < 3000 && pinky < 1000){
          document.getElementById("myElement").textContent = 'k';
        }
        else if (thumb > 2000 && index > 3000 && middle < 500 && ring < 1000 && pinky < 500){
          document.getElementById("myElement").textContent = 'l';
        }
        else if (thumb < 400 && index < 800 && middle < 400 && ring < 1000 && pinky < 400){
          document.getElementById("myElement").textContent = 'm';
        }
        else if (thumb < 100 && index < 800 && middle < 100 && ring < 1200 && pinky < 400 && ring > 500){
          document.getElementById("myElement").textContent = 'm';
        }
        else if (thumb < 300 && index < 3000 && middle < 300 && ring < 1800 && pinky < 600 && index > 1500){
          document.getElementById("myElement").textContent = 'n';
        }
        else if (thumb > 3000 && index > 3000 && middle > 500 && ring < 1500 && pinky < 600 && middle < 1500){
          document.getElementById("myElement").textContent = 'p';
        }
        else if (thumb > 3500 && index > 2000 && middle < 100 && ring < 1000 && pinky < 500 && index < 4000){
          document.getElementById("myElement").textContent = 'q';
        }
         else if (thumb < 4000 && index < 1500 && middle > 3000 && ring < 2500 && pinky < 900){
          document.getElementById("myElement").textContent = 'r';
        }
         else if (thumb < 3000 && index < 500 && middle < 100 && ring < 1000 && pinky < 300 && thumb > 1000){
          document.getElementById("myElement").textContent = 's';
        }
        else if (thumb < 1000 && index < 3000 && middle < 300 && ring < 1500 && pinky < 300 && index > 1000){
          document.getElementById("myElement").textContent = 't';
        }
        else if (thumb < 3000 && index > 3000 && middle > 3000 && ring < 600 && pinky < 600){
          document.getElementById("myElement").textContent = 'u';
        }
        else if (thumb < 2500 && index > 3000 && middle < 3000 && ring < 1500 && pinky < 600 && middle > 2000){
          document.getElementById("myElement").textContent = 'v';
        }
        else if (thumb < 2500 && index > 3000 && middle > 3000 && ring > 3000 && pinky < 600){
          document.getElementById("myElement").textContent = 'w';
        }
         else if (thumb < 2000 && index > 1500 && middle < 100 && ring < 1000 && pinky < 600 && index < 4095){
          document.getElementById("myElement").textContent = 'x';
        }
         else if (thumb > 3000 && index < 2500 && middle < 1000 && ring < 1000 && pinky > 3000){
          document.getElementById("myElement").textContent = 'y';
        }
        else {
         document.getElementById("myElement").textContent = '';
         }
     }

    function checkGesture(index, thumb, middle, ring, pinky) {
        if (currentLetter) {
            var ranges = letterRanges[currentLetter];
            var result = 'Letter ' + currentLetter + ': ';

            if (thumb >= ranges.thumb[0] && thumb <= ranges.thumb[1]) {
                result += 'Thumb: Y, ';
                document.getElementById("thumb_fix").textContent = '';
            } 
            else {
                result += 'Thumb: N, ';
                if (thumb <= ranges.thumb[0]){
                  document.getElementById("thumb_fix").textContent = 'unbend your thumb slightly';
                }
                else if (thumb >= ranges.thumb[1]){
                  document.getElementById("thumb_fix").textContent = 'bend your thumb slightly';
                }
            }

            if (index >= ranges.index[0] && index <= ranges.index[1]) {
                result += 'Index: Y, ';
                document.getElementById("index_fix").textContent = '';
            } else {
                result += 'Index: N, ';
                if (index <= ranges.index[0]){
                  document.getElementById("index_fix").textContent = 'unbend your index slightly';
                }
                else if (index >= ranges.index[1]){
                  document.getElementById("index_fix").textContent = 'bend your index slightly';
                }

            }

            if (middle >= ranges.middle[0] && middle <= ranges.middle[1]) {
                result += 'Middle: Y, ';
                document.getElementById("middle_fix").textContent = '';
            } else {
                result += 'Middle: N, ';
                if (middle <= ranges.middle[0]){
                  document.getElementById("middle_fix").textContent = 'unbend your middle slightly';
                }
                else if (middle >= ranges.middle[1]){
                  document.getElementById("middle_fix").textContent = 'bend your middle slightly';
                }
          
            }

            if (ring >= ranges.ring[0] && ring <= ranges.ring[1]) {
                result += 'Ring: Y, ';
                document.getElementById("ring_fix").textContent = '';
            } else {
                result += 'Ring: N, ';
                if (ring <= ranges.ring[0]){
                  document.getElementById("ring_fix").textContent = 'unbend your ring slightly';
                }
                else if (ring >= ranges.ring[1]){
                  document.getElementById("ring_fix").textContent = 'bend your ring slightly';
                }

            }

            if (pinky >= ranges.pinky[0] && pinky <= ranges.pinky[1]) {
                result += 'Pinky: Y';
                document.getElementById("pinky_fix").textContent = '';
            } else {
                result += 'Pinky: N';
                if (pinky <= ranges.pinky[0]){
                  document.getElementById("pinky_fix").textContent = 'unbend your pinky slightly';
                }
                else if (pinky >= ranges.pinky[1]){
                  document.getElementById("pinky_fix").textContent = 'bend your pinky slightly';
                }
            }

            document.getElementById('quizResult').textContent = result;
            
        }
    }
</script>

</body>
</html>


)=====";


#include <ESPAsyncWebServer.h>

AsyncWebServer server(80); // server port 80
WebSocketsServer websockets(81);

void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Page Not found");
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

  switch (type) 
  {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {
        IPAddress ip = websockets.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        // send message to client
        websockets.sendTXT(num, "Connected from server");
      }
      break;
    case WStype_TEXT: 
      Serial.printf("[%u] get Text: %s\n", num, payload);
      String message = String((char*)( payload));
      Serial.println(message);
    
     DynamicJsonDocument doc(200);
    // deserialize the data
    DeserializationError error = deserializeJson(doc, message);
    // parse the parameters we expect to receive (TO-DO: error handling)
      // Test if parsing succeeds.
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }
  
}
 
}

void send_sensor(){
  int flexValue1 = analogRead(flexPin1);
  int flexValue2 = analogRead(flexPin2);
  int flexValue3 = analogRead(flexPin3);
  int flexValue4 = analogRead(flexPin4);
  int flexValue5 = analogRead(flexPin5);

  //scale values
  int scaledflex1 = map(flexValue1, 0, 4095, 0, 100);
  int scaledflex2 = map(flexValue2, 0, 4095, 0, 100);
  int scaledflex3 = map(flexValue3, 0, 4095, 0, 100);
  int scaledflex4 = map(flexValue4, 0, 4095, 0, 100);
  int scaledflex5 = map(flexValue5, 0, 4095, 0, 100);

 if (isnan(flexValue1) || isnan(flexValue2) || isnan(flexValue3) || isnan(flexValue4) || isnan(flexValue5)) {
    Serial.println(F("Failed to read from Flex sensors!"));
    return;
 }
 
//JSON_Data
   String JSON_Data = "{\"flex1\":";
         JSON_Data += scaledflex1;
         JSON_Data += ",\"flex2\":";
         JSON_Data += scaledflex2;
         JSON_Data += ",\"flex3\":";
         JSON_Data += scaledflex3;
         JSON_Data += ",\"flex4\":";
         JSON_Data += scaledflex4;
         JSON_Data += ",\"flex5\":";
         JSON_Data += scaledflex5;
         JSON_Data += "}";
   Serial.println(JSON_Data);     
  websockets.broadcastTXT(JSON_Data);


}

void setup(void)
{
  
  Serial.begin(115200);
  pinMode(flexPin1, INPUT);
  pinMode(flexPin2, INPUT);
  pinMode(flexPin3, INPUT);
  pinMode(flexPin4, INPUT);
  pinMode(flexPin5, INPUT);

  WiFi.softAP("techiesms", "");
  Serial.println("softap");
  Serial.println("");
  Serial.println(WiFi.softAPIP());


  if (MDNS.begin("ESP")) { //esp.local/
    Serial.println("MDNS responder started");
  }

  server.on("/", [](AsyncWebServerRequest * request)
  { 
   
  request->send_P(200, "text/html", webpage);
  });

  server.on("/learn", [](AsyncWebServerRequest * request)
  { 
   
  request->send_P(200, "text/html", learnpage);
  });

  server.begin();  // it will start webserver
  websockets.begin();
  websockets.onEvent(webSocketEvent);
  timer.attach(.1, send_sensor);
}


void loop(void)
{
  websockets.loop();
}
  // int flexValue1 = analogRead(flexPin1);
  // Serial.print("Index: ");
  // Serial.println(flexValue1);
  // int flexValue3 = analogRead(flexPin3);
  // Serial.print("Middle: ");
  // Serial.println(flexValue3);
  // int flexValue5 = analogRead(flexPin5);
  // Serial.print("Ring: ");
  // Serial.println(flexValue5);
  // int flexValue4 = analogRead(flexPin4);
  // Serial.print("Pinky: ");
  // Serial.println(flexValue4);
  // int flexValue2 = analogRead(flexPin2);
  // Serial.print("Thumb: ");
  // Serial.println(flexValue2);

  //  delay(50);




