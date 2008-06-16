<?php
header('Content-Type: text/html; charset=UTF-8');

// ruta completa del fichero de configuración
define("CONF_FILE", './dhcpd.conf');


function saveConfig($param1, $param2, $param3, $param4, $param5) {
  unlink(CONF_FILE);
  $fh = fopen(CONF_FILE, 'a+') or die("Error!!");
  fwrite($fh, "$param1\t");
  fwrite($fh, "$param2\t"); 
  fwrite($fh, "$param3\t");
  fwrite($fh, "$param4\t");
  fwrite($fh, "$param5\t");
  fclose($fh);
}


function listConfig() {
  $handle = fopen(CONF_FILE, "a+");

  while ($config = fscanf($handle, "%s\t%s\t%s\t%s\t%s\n")) {
    list ($param1, $param2, $param3, $param4, $param5) = $config;
    echo '<tr><td>DNS:</td><td><input type="text" name="param1" size="15" value="' . $param1 . '"></td></tr>';
    echo '<tr><td>Subnet:</td><td><input type="text" name="param2" size="15" value="' . $param2 . '"></td></tr>';
    echo '<tr><td>Rango:</td><td><input type="text" name="param3" size="15" value="' . $param3 . '"> <input type="text" name="param4" size="15" value="' . $param4 . '"></td></tr>';
    echo '<tr><td>Puerta de Enlace:</td><td><input type="text" name="param5" size="15" value="' . $param5 . '"></td></tr>';
  }

  fclose($handle);
}

if (!empty($_REQUEST["op"])) {
  switch ($_REQUEST["op"]) {
    case 'change':
      saveConfig($_REQUEST["param1"], $_REQUEST["param2"], $_REQUEST["param3"], $_REQUEST["param4"], $_REQUEST["param5"]);
      //exec('loquesea');
      exec('./script/smb.sh');
      break;
  }
}

?>

<html>
  <head>
    <title>MAXControl - Configuración del Servidor de DHCP</title>
    <link href="ultramarine.css" rel="stylesheet" type="text/css">
    <style type="text/css">
      body {	background-image: url("./imagenes/max.jpg");
		background-position: 100% 20%}
      .xtb    { margin-top:     2.08em;
	        margin-bottom:  .75em;
        	padding-top:    .08em;
        	padding-bottom: 0;
        	border-top:     .1em;
        	border-bottom:  0;
        	border-style:   solid }
      h1, h2, h3, h4, h5, h6, dt, th, thead, tfoot    {
      		color:  #888000; }

    </style>
  </head>
  <body>
    <form name="recursos" action="<?php echo "http://" . $_SERVER['SERVER_NAME'] . $_SERVER['REQUEST_URI']; ?>" method="post">
      <p class="xtb"><h2>Configuración del Servidor de DHCP</h2>
        <br><table>
          <?php listConfig(); ?>
          </td><td></td></tr>
        </table>
      </p><p>
        <br><input type="submit" value="Guardar">
        &nbsp;&nbsp;<input type="reset" value="Limpiar">
      </p>
      <input type="hidden" name="op" id="op" value="change">
    </form>
  </body>
</html>

