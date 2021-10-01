<?php 

// Camera
$camera = (isset($_GET['camera']) ? $_GET['camera'] : false);
$dag    = (isset($_GET['dag'])    ? $_GET['dag']    : false);
$bart   = (isset($_GET['bart'])   ? $_GET['bart']   : false);
$path   = $camera.'/'.$dag.'/'.$bart.'/';
$images = scandir($path);

$date = array("dag" => date('N'), "dagdeel" => date('a'));

?><!DOCTYPE html>
<html lang="en">
<head>
<title>Schans archief: <?php echo $camera; ?></title>
<style type="text/css">
	body { height: 100%; margin: 0; font: bold 24px Arial, sans-serif; background: black; color: white; }
	a { color: #FFFFFF; text-decoration: none; }
	img { float: left; padding: 1px; width: 340px; }
	div#page1, div#page2, div#page3, div#page4 { height: 100%; }
	div#page1 div, div#page2 div, div#page3 div { width: 100%; border-bottom: 2px solid #FFFFFF; padding: 2% 0; text-indent: 3%; }
	div#page1 div a:hover, div#page2 div a:hover, div#page3 div a:hover { color: #CCCCCC; }
	div#layer { width: 100%; height: 100%; display: none; position: fixed; top: 0px; left: 0px; z-index: 100; }
	div.imagediv { position: relative; width: 347px; height: 257px; }
	img.image { position: absolute; top: 0px; left: 0px; border: 10px #00ff00"; }
</style>
<script>

function displayImage(img) 
{ document.getElementById("layer").innerHTML = '<img src="'+img+'" style="width:66%; border: 10px solid black;"; />'; 
  document.getElementById('layer').style.display = 'block'; } 

</script>
</head>
<body>
<div id="layer" onclick="this.innerHTML = ''; this.style.display = 'none';"></div>

<?php 

	if(!$camera)
	{
		echo '<div id="page1">
		<div><a href="?camera=kalverstal">Kalverstal</a></div>
		<div><a href="?camera=smederij">Smederij</a></div>
		<div><a href="?camera=robot1">Robot 1</a></div>
		<div><a href="?camera=robot2">Robot 2</a></div>
		</div>'; 
	}
	else if($camera && !$dag)
	{
		echo '<div id="page2">'.$camera.'
		<div><a '.($date['dag'] == 7 ? "style='color: red;'" : "").' href="?camera='.$camera.'&dag=zondag">Zondag</a></div>
		<div><a '.($date['dag'] == 1 ? "style='color: red;'" : "").' href="?camera='.$camera.'&dag=maandag">Maandag</a></div>
		<div><a '.($date['dag'] == 2 ? "style='color: red;'" : "").' href="?camera='.$camera.'&dag=dinsdag">Dinsdag</a></div>
		<div><a '.($date['dag'] == 3 ? "style='color: red;'" : "").' href="?camera='.$camera.'&dag=woensdag">Woensdag</a></div>
		<div><a '.($date['dag'] == 4 ? "style='color: red;'" : "").' href="?camera='.$camera.'&dag=donderdag">Donderdag</a></div>
		<div><a '.($date['dag'] == 5 ? "style='color: red;'" : "").' href="?camera='.$camera.'&dag=vrijdag">Vrijdag</a></div>
		<div><a '.($date['dag'] == 6 ? "style='color: red;'" : "").' href="?camera='.$camera.'&dag=zaterdag">Zaterdag</a></div>
		<div><a href="?camera='.$camera.'">Terug naar '.$camera.'</a></div>
		<div><a href="?">Terug naar begin</a></div>
		</div>';
	}
	else if ($camera && $dag && !$bart)
	{
		echo '<div id="page3">'.$camera.'
		<div><a '.($date['dagdeel'] == "am" ? "style='color:red;'" : "").' href="?camera='.$camera.'&dag='.$dag.'&bart=am">AM</a></div>
		<div><a '.($date['dagdeel'] == "pm" ? "style='color:red;'" : "").' href="?camera='.$camera.'&dag='.$dag.'&bart=pm">PM</a></div>
		<div><a href="?camera='.$camera.'">Terug naar '.$camera.'</a></div>
		<div><a href="?">Terug naar begin</a></div>
		</div>';
	}
	else if($camera && $dag && $bart)
	{
		echo '<div id="page4">';

        for($i=(count($images) - 1);$i>1;$i--) 
        {
        	if(!strpos($images[$i], 'shadow')) 
        	{ echo '
        		<div style="float:left; width: 347px; height: 257px;"><div class="imagediv">
        			<img src="'.$path.$images[$i].'" onclick="displayImage(\''.$path.$images[$i].'\')" class="image" />
        		</div></div>'; }
		}
		echo '</div>';
	}
?>
</body>
</html>
