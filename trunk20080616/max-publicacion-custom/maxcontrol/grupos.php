<?php
header('Content-Type: text/html; charset=UTF-8');

// ruta completa del fichero de configuración
define("CONF_FILE", '/tmp/test.conf');

define("USER_FILTER", "getent passwd|cut -d ':' -f1|grep -v -f ./usuarios_bloqueados.txt");

// comando para extraer los grupos del sistema
define("GROUP_FILTER", "getent group|cut -d ':' -f1|grep -v -f ./grupos_bloqueados.txt");


function obtainUsers() {
  $res = array();
  
  exec(GROUP_FILTER, $res);
  
  foreach ($res as $k => $v) {
    echo '<br /><input type="radio" name="param1" value="' . $v .'"> ' . $v . '</input>';
  }
}

//-----
function obtainGroups() {
  $res = array();
  
  exec(GROUP_FILTER, $res);
  
  foreach ( $res as $k => $v) {
    echo '<option value="' . $v . '">' . $v . '</option>';
  }
}


function addResource($param1, $param2) {
  $fh = fopen(CONF_FILE, 'a+') or die("Error!!");
  fwrite($fh, "$param1\t");
  fwrite($fh, "$param2\t"); 
  fclose($fh);
}


function delResource($param1) {
  $regs = array();
  //$handle = fopen(CONF_FILE, "r");
  $fh = fopen(CONF_FILE, "r");

  //while ($userinfo = fscanf($handle, "%s\t%s\t%s\n")) {
  while ($userinfo = fscanf($fh, "%s\t%s\t%s\n")) {
    $regs[] = $userinfo;
  }
  
  fclose($fh);
  
  unlink(CONF_FILE);

  foreach($regs as $key => $value) {
    if ($value[0] != $param1) {
      addResource($value[0], $value[1], $value[2]);
    }
  }
}


function listResources() {
  $handle = fopen(CONF_FILE, "a+");

  while ($userinfo = fscanf($handle, "%s\t%s\t%s\n")) {
    list ($param1, $param2, $param3) = $userinfo;
    echo '<br /><input type="checkbox" name="param1" value="' . $param1 .'"> ' . $param1 .' -- ' . $param2 .' -- ' . $param3;
  }

  fclose($handle);
}

if (!empty($_REQUEST["op"])) {
  switch ($_REQUEST["op"]) {
    case 'add':
      exec("sudo /usr/sbin/groupadd ". $_REQUEST['param1']);
      break;
    case 'delete':
      exec("sudo /usr/sbin/groupdel " . $_REQUEST['param1']);
      break;
  }
}

?>

<html>
  <head>
    <title>MAXControl - Grupos del Sistema</title>
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
    <form name="recursos" action="<?php echo "http://" . $_SERVER['SERVER_NAME'] . $_SERVER['REQUEST_URI']; ?>" METHOD="post">
      <p>
        <h1>Grupos</h1>
          <?php obtainUsers(); ?>
        <br><input type="submit" value="Borrar">
      </p>
      <input type="hidden" name="op" id="op" value="delete">
    </form>
    <form name="recursos" action="<?php echo "http://" . $_SERVER['SERVER_NAME'] . $_SERVER['REQUEST_URI']; ?>" method="post">
      <p class="xtb"><h2>Nuevo Grupo</h2>
        <br><table>
          <tr><td>Nombre del Grupo:</td><td><input type="text" name="param1" size="15"></td></tr>
          </td><td></td></tr>
        </table>
      </p><p>
        <br><input type="submit" value="Guardar">
        &nbsp;&nbsp;<input type="reset" value="Limpiar">
      </p>
      <input type="hidden" name="op" id="op" value="add">
    </form>
  </body>
</html>

