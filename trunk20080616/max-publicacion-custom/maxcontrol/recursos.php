<?php
header('Content-Type: text/html; charset=UTF-8');

// ruta completa del fichero de configuración
define("CONF_FILE", './recursos.conf');

// comando para extraer los grupos del sistema
define("GROUP_FILTER", "getent group|cut -d ':' -f1|grep -v -f ./grupos_bloqueados.txt");


function obtainGroups() {
  $res = array();
  
  exec(GROUP_FILTER, $res);
  
  foreach ( $res as $k => $v) {
    echo '<option value="' . $v . '">' . $v . '</option>';
  }
}


function addResource($param1, $param2, $param3, $param4) {
  $fh = fopen(CONF_FILE, 'a+') or die("Error!!");
  fwrite($fh, "$param1\t");
  fwrite($fh, "$param2\t");
  fwrite($fh, "$param3\t"); 
  fwrite($fh, "$param4\n"); 
  fclose($fh);
}


function delResource($param1) {
  $regs = array();
  //$handle = fopen(CONF_FILE, "r");
  $fh = fopen(CONF_FILE, "r");

  //while ($userinfo = fscanf($handle, "%s\t%s\t%s\n")) {
  while ($userinfo = fscanf($fh, "%s\t%s\t%s\t%s\n")) {
    $regs[] = $userinfo;
  }
  
  fclose($fh);
  
  unlink(CONF_FILE);

  foreach($regs as $key => $value) {
    if ($value[0] != $param1) {
      addResource($value[0], $value[1], $value[2], $value[3]);
    }
  }
}


function listResources() {
  $handle = fopen(CONF_FILE, "a+");

  while ($userinfo = fscanf($handle, "%s\t%s\t%s\t%s\n")) {
    list ($param1, $param2, $param3, $param4) = $userinfo;
    echo '<br /><input type="radio" name="param1" value="' . $param1 .'"> ' . $param1 .' : ' . $param2 .' : ' . $param3 .' : ' . $param4;
  }

  fclose($handle);
}

if (!empty($_REQUEST["op"])) {
  switch ($_REQUEST["op"]) {
    case 'add':
      addResource($_REQUEST["param1"], $_REQUEST["param2"], $_REQUEST["param3"], $_REQUEST["param4"]);
      exec("./scripts/samba.sh");
      break;
    case 'delete':
      delResource($_REQUEST["param1"]);
      break;
  }
}

?>

<html>
  <head>
    <title>MAXControl - Recursos Compartidos</title>
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
        <h1>Recursos Compartidos</h1>
          <?php listResources(); ?>
        <br><input type="submit" value="Borrar">
      </p>
      <input type="hidden" name="op" id="op" value="delete">
    </form>
    <form name="recursos" action="<?php echo "http://" . $_SERVER['SERVER_NAME'] . $_SERVER['REQUEST_URI']; ?>" method="post">
      <p class="xtb"><h2>Nuevo Recurso</h2>
        <br><table>
          <tr><td>Nombre:</td><td><input type="text" name="param1" size="50"></td></tr>
          <tr><td>Recurso:</td><td><input type="text" name="param2" size="50"></td></tr>
          <tr><td>Acceso:</td><td><input type="radio" name="param3" value="r">Sólo Lectura<input type="radio" name="param3" value="w">Lectura y Escritura</td></tr>
          <tr><td>Grupo:</td><td><select name="param4">
          <?php obtainGroups(); ?>
          </select>
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

