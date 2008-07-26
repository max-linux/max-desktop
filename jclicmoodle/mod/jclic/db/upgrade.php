<?php  //$Id: upgrade.php,v 1.1 2007/10/03 09:33:32 sarjona Exp $

// This file keeps track of upgrades to 
// the jclic module
//
// Sometimes, changes between versions involve
// alterations to database structures and other
// major things that may break installations.
//
// The upgrade function in this file will attempt
// to perform all the necessary actions to upgrade
// your older installtion to the current version.
//
// If there's something it cannot do itself, it
// will tell you what you need to do.
//
// The commands in here will all be database-neutral,
// using the functions defined in lib/ddllib.php

function xmldb_jclic_upgrade($oldversion=0) {
    global $CFG;

    $result = true;
    return $result;
}

?>
