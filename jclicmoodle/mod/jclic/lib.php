<?PHP  // $Id: lib.php,v 1.6 2008/01/24 15:30:42 sarjona Exp $

/// Library of functions and constants for module jclic

require_once($CFG->libdir.'/pagelib.php');


if (!isset($CFG->jclic_jclicpluginjs)) {
    set_config("jclic_jclicpluginjs", "http://clic.xtec.net/dist/jclic/jclicplugin.js");
}
if (!isset($CFG->jclic_lap)) {
    set_config("jclic_lap", "5");
}

if (function_exists('date_default_timezone_set')) date_default_timezone_set('UTC');


function jclic_add_instance($jclic) {
/// Given an object containing all the necessary data, 
/// (defined by the form in mod.html) this function 
/// will create a new instance and return the id number 
/// of the new instance.
    if ($jclic->skin=='') $jclic->skin="default";

    return insert_record("jclic", $jclic);
}


function jclic_update_instance($jclic) {
/// Given an object containing all the necessary data, 
/// (defined by the form in mod.html) this function 
/// will update an existing instance with new data.
    $jclic->id = $jclic->instance;

    return update_record("jclic", $jclic);
}


function jclic_delete_instance($id) {
/// Given an ID of an instance of this module, 
/// this function will permanently delete the instance 
/// and any data that depends on it.


    if (! $jclic = get_record("jclic", "id", "$id")) {
        return false;
    }
    $result = true;
    
    # Delete any dependent records here #
    $rs =  get_record("jclic_sessions", "jclicid", "$id");
    foreach($rs as $session){
      delete_records("jclic_activities", "session_id", "$rs->session_id");
	  }

    delete_records("jclic_sessions", "jclicid", "$id");
    
    if (! delete_records("jclic", "id", "$id")) {
        $result = false;
    }
    
    
    return $result;
}

function jclic_user_outline($course, $user, $mod, $jclic) {
/// Return a small object with summary information about what a 
/// user has done with a given particular instance of this module
/// Used for user activity reports.
/// $return->time = the time they did it
/// $return->info = a short text description

    return $return;
}

function jclic_user_complete($course, $user, $mod, $jclic) {
/// Print a detailed representation of what a  user has done with 
/// a given particular instance of this module, for user activity reports.

    return true;
}

function jclic_print_recent_activity($course, $isteacher, $timestart) {
/// Given a course and a time, this module should find recent activity 
/// that has occurred in jclic activities and print it out.
/// Return true if there was output, or false is there was none.

    global $CFG;

    return false;  //  True if anything was printed, otherwise false 
}

function jclic_cron () {
/// Function to be run periodically according to the moodle cron
/// This function searches for things that need to be done, such 
/// as sending out mail, toggling flags etc ... 

    global $CFG;

    return true;
}

function jclic_grades($jclicid) {
	global $CFG;
/// Must return an array of grades for a given instance of this module, 
/// indexed by user.  It also returns a maximum allowed grade.
///
///    $return->grades = array of grades;
///    $return->maxgrade = maximum allowed grade;
///

    if (!$jclic = get_record('jclic', 'id', $jclicid)) {
        return NULL;
    }
    if ($jclic->maxgrade == null) { // No grading
        return NULL;
    }

    $return = new object();
  	 if ($participants = jclic_get_participants($jclicid)){
  		foreach ($participants as $participant) {
  			$sessions_summary=jclic_get_sessions_summary($jclicid, $participant->userid);
			if ($jclic->avaluation=='score'){
				$grades[$participant->userid]=$sessions_summary->score;				
			}else{
				$grades[$participant->userid]=$sessions_summary->solved;
			}
  		}
  	}
  	$return->grades=$grades;
  	$return->maxgrade=$jclic->maxgrade;
   return $return;


/*	$grades = array();
	$jclic = get_record("jclic", "id", "$jclicid");
	if ($students = jclic_get_students($jclicid)){
		foreach ($students as $student) {
			$summary_sessions=jclic_get_sessions_summary($jclicid, $student->userid);
			if ($jclic->avaluation=='score'){
				$grades[$student->userid]=$summary_sessions->score;				
			}else{
				$grades[$student->userid]=$summary_sessions->solved;
			}
		}
	}
	$return->grades=$grades;
	$return->maxgrade=$jclic->maxgrade;
	
	return $return;*/
}

function jclic_get_students($cm, $course, $jclicid) {
	global $CFG;
	$version_moodle = (float)substr($CFG->release,0,3);
	if ($version_moodle>=1.7){
    $context = get_context_instance(CONTEXT_MODULE, $cm->id);
    $currentgroup = get_and_set_current_group($course, groupmode($course, $cm));
    $users = get_users_by_capability($context, 'mod/jclic:submit', 'u.id, u.id', '', '', '', $currentgroup, '', false);
    $select = 'SELECT u.id AS userid, u.firstname, u.lastname, u.picture ';
    $sql = 'FROM '.$CFG->prefix.'user u '.
           'LEFT JOIN '.$CFG->prefix.'jclic_sessions a ON u.id = a.user_id AND a.jclicid = '.$jclicid.' '.
           'WHERE u.id IN ('.implode(',', array_keys($users)).') ';               
    $dbstudents=get_records_sql($select.$sql);
    
/*	 $dbstudents = get_records_sql("SELECT DISTINCT u.id AS userid, u.firstname, u.lastname
                                  FROM {$CFG->prefix}user u LEFT JOIN (({$CFG->prefix}role_assignments r
                                  LEFT JOIN {$CFG->prefix}user_lastaccess ul ON r.userid = ul.userid)
                                  LEFT JOIN {$CFG->prefix}jclic j ON ul.courseid = j.course) ON u.id = r.userid
                                  WHERE (((u.deleted)=0) AND ((u.username)<>'guest') AND j.id=$jclicid AND ((r.roleid) NOT IN (1) AND (r.roleid)=5))");	*/
  }else{
  	$dbstudents = get_records_sql("SELECT DISTINCT us.userid, u.firstname, u.lastname
  				       FROM {$CFG->prefix}user u,{$CFG->prefix}user_students us, {$CFG->prefix}jclic j
  				       WHERE us.course=j.course AND j.id=$jclicid AND u.id=us.userid");
  }
	
	return $dbstudents;
}

function jclic_get_participants($jclicid) {
//Must return an array of user records (all data) who are participants
//for a given instance of jclic. Must include every user involved
//in the instance, independient of his role (student, teacher, admin...)
//See other modules as example.

    global $CFG;

    //Get students
    $students = get_records_sql("SELECT DISTINCT u.id, u.id as userid
                                 FROM {$CFG->prefix}user u,
                                      {$CFG->prefix}jclic_sessions js
                                 WHERE js.jclicid = '$jclicid' and
                                       u.id = js.user_id");
    //Get teachers
    $teachers = get_records_sql("SELECT DISTINCT u.id, u.id as userid
                                 FROM {$CFG->prefix}user u,
                                      {$CFG->prefix}jclic_sessions js
                                 WHERE js.jclicid = '$jclicid' and
                                       u.id = js.user_id");

    //Add teachers to students
    if ($teachers) {
        foreach ($teachers as $teacher) {
            $students[$teacher->id] = $teacher;
        }
    }
    //Return students array (it contains an array of unique users)
    return ($students);
}

function jclic_scale_used ($jclicid,$scaleid) {
//This function returns if a scale is being used by one jclic
//it it has support for grading and scales. Commented code should be
//modified if necessary. See forum, glossary or journal modules
//as reference.
   
    $return = false;

    return $return;
}

//////////////////////////////////////////////////////////////////////////////////////

/**
* Get user sessions
*
* @return array			[0=>session1,1=>session2...] where session1 is an array with keys: id,score,totaltime,starttime,done,solved,attempts. First sessions are newest.
* @param object $jclicid	The jclic to get sessions
* @param object $userid		The user id to get sessions
*/
function jclic_get_sessions($jclicid, $userid) {
	global $CFG;

  $sessions=array();
	if($rs = get_records_sql("SELECT js.*
                            FROM {$CFG->prefix}jclic j, {$CFG->prefix}jclic_sessions js 
                            WHERE j.id=js.jclicid AND js.jclicid=$jclicid AND js.user_id=$userid
                            ORDER BY js.session_datetime")){
		$i=0;
		foreach($rs as $session){
			$activity=jclic_get_activity($session);
			$activity->attempts=$i+1;
			$sessions[$i++]=$activity;
		}
	}
	return $sessions;
}

/**
* Get session activities
*
* @return array			[0=>act0,1=>act1...] where act0 is an array with keys: activity_id,activity_name,num_actions,score,activity_solved,qualification, total_time. First activity are oldest.
* @param string $session_id		The session id to get actitivies
*/
function jclic_get_activities($session_id) {
	global $CFG;
	
	if($rs = get_records("jclic_activities", "session_id", "$session_id",'activity_id')){
		$i=0;
		foreach($rs as $activity){
			$activities[$i++]=$activity;
		}
	}
	return $activities;
}

/**
* Get information about activities of specified session
*
* @return array			Array has these keys id,score,totaltime,starttime,done,solved,attempts
* @param object $session	The session object
*/
function jclic_get_activity($session) {
	global $CFG;
	$activity->starttime=$session->session_datetime;
	$activity->session_id=$session->session_id;
	if($rs = get_record_sql("SELECT AVG(ja.qualification) as qualification, SUM(ja.total_time) as totaltime
    				 FROM {$CFG->prefix}jclic_activities ja 
				 WHERE ja.session_id='$session->session_id'")){
		$activity->score=round($rs->qualification,0);
		$activity->totaltime=round($rs->totaltime/60000,0)."' ".round(fmod($rs->totaltime,60000)/1000,0)."''";
	}
	if ($rs = get_record_sql("SELECT COUNT(*) as done
                            FROM (SELECT DISTINCT ja.activity_name 
                                  FROM  {$CFG->prefix}jclic_activities ja 
                                  WHERE ja.session_id='$session->session_id') t")){
    $activity->done=$rs->done;
  }
	if ($rs = get_record_sql("SELECT COUNT(*) as solved
                            FROM (SELECT DISTINCT ja.activity_name 
                                  FROM {$CFG->prefix}jclic_activities ja 
                                  WHERE ja.session_id='$session->session_id' AND ja.activity_solved=1) t")){
    $activity->solved=$rs->solved;
	}

	return $activity;
}

/**
* Get user activity summary
*
* @return object	session object with score, totaltime, activities done and solved and attempts information
*/
function jclic_get_sessions_summary($jclicid, $userid) {
	global $CFG;

  $sessions_sumari = array('attempts'=>'','score'=>'','totaltime'=>'','starttime'=>'','done'=>'','solved'=>'');
	if ($rs = get_record_sql("SELECT COUNT(*) AS attempts, AVG(t.qualification) AS qualification, SUM(t.totaltime) AS totaltime, MAX(t.starttime) AS starttime
                            FROM (SELECT AVG(ja.qualification) AS qualification, SUM(ja.total_time) AS totaltime, MAX(js.session_datetime) AS starttime
                                  FROM {$CFG->prefix}jclic j, {$CFG->prefix}jclic_sessions js, {$CFG->prefix}jclic_activities ja  
                                  WHERE j.id=js.jclicid AND js.user_id=$userid AND js.jclicid=$jclicid AND ja.session_id=js.session_id
                                  GROUP BY js.session_id) t")){
		$sessions_summary->attempts=$rs->attempts;
		$sessions_summary->score=round($rs->qualification,0);
		$sessions_summary->totaltime=round($rs->totaltime/60000,0)."' ".round(fmod($rs->totaltime,60000)/1000,0)."''";
		$sessions_summary->starttime=$rs->starttime;
	}
	
	if ($rs = get_record_sql("SELECT COUNT(*) as done
                            FROM (SELECT DISTINCT ja.activity_name 
                                  FROM {$CFG->prefix}jclic j, {$CFG->prefix}jclic_sessions js, {$CFG->prefix}jclic_activities ja 
                                  WHERE j.id=js.jclicid AND js.user_id=$userid AND js.jclicid=$jclicid AND js.session_id=ja.session_id)  t")){
		$sessions_summary->done=$rs->done;
	}
	if ($rs = get_record_sql("SELECT COUNT(*) as solved
                            FROM (SELECT DISTINCT ja.activity_name 
                                  FROM {$CFG->prefix}jclic j, {$CFG->prefix}jclic_sessions js, {$CFG->prefix}jclic_activities ja 
                                  WHERE j.id=js.jclicid AND js.user_id=$userid AND js.jclicid=$jclicid AND js.session_id=ja.session_id AND ja.activity_solved=1) t")){
    $sessions_summary->solved=$rs->solved;
	}
	return $sessions_summary;
}

/**
* Format time from milliseconds to string 
*
* @return string Formated string [x' y''], where x are the minutes and y are the seconds.	
* @param int $time	The time (in ms)
*/
function jclic_time2str($time){
  return round($time/60000,0)."' ".round(fmod($time,60000)/1000,0)."''";
}

/**
* Print data in array as a row. Uses jclic_make_row function to get the string representation to print
*
* @param array $row Data
* @param array $align Cell alignment
* @param array $props Style, rowspan, colspan...
* @param boolean $header If true the row will be printed as th; otherwise as td.
*/
function jclic_print_row($row, $align, $props=array(), $header=false){
  echo jclic_make_row($row, $align, $props, $header);
}

function jclic_make_row($row, $row_align, $row_props=array(), $header=false){
  $strrow = '';
  $strtd = $header?'th':'td';
  if (sizeof($row)>0){
    $strrow.='<tr>';
    $i=0;
		foreach($row as $cell){
		  $prop = ''; $align='';
		  if ($row_props!='' && array_key_exists($i, $row_props) && $row_props[$i]!='') $prop = ' '.$row_props[$i].' ';
		  if ($row_align!=''&& array_key_exists($i, $row_align) && $row_align[$i]!='') $align = ' align='.$row_align[$i].' ';
      $strrow.='<'.$strtd.' '.($header?'':'class="cell"').$prop.$align.'>'.$cell.'</'.$strtd.'>';
      $i++;	  
    }
    $strrow.='</tr>';
  }  
  return $strrow;
}

/**
* Print a table data with all session activities 
* 
* @param string $session_id The session identifier
*/
function print_session_activities($session_id){
  // Import language strings
  $stractivity = get_string("activity", "jclic");
  $strsolved = get_string("solved", "jclic");
  $stractions = get_string("actions", "jclic");
  $strtime = get_string("time", "jclic");
  $strscore  = get_string("score", "jclic");
  $stryes = get_string("yes");
  $strno = get_string("no");
  
  // Print activities for each session
  $activities=jclic_get_activities($session_id);    
	if (sizeof($activities)>0){
	  echo '<tr><td colspan="7"><div id="'.$session_id.'" style="display:none;visible:hidden">';
	  echo '<table class="generaltable" align="center" border="0" cellpadding="5" cellspacing="1" width="80%"><tbody>';
	  $subtable_align=array('left','center','center','center','center');
	  jclic_print_row(array($stractivity, $strsolved, $stractions, $strtime, $strscore),$subtable_align,'',true);
		foreach($activities as $activity){
		  $subtable_props=array('', 'style="background-color:'.($activity->activity_solved?'#DFFFDF':'#FFDFDF').'"','','','');
		  $act_percent=$activity->num_actions>0?round(($activity->score/$activity->num_actions)*100,0):0;
		  jclic_print_row(array($activity->activity_name, ($activity->activity_solved?$stryes:$strno), $activity->score.'/'.$activity->num_actions.' ('.$act_percent.'%)', jclic_time2str($activity->total_time), $activity->qualification.'%'), $subtable_align, $subtable_props);
		}
		echo '</table></div></td></tr>';
  }
}

function jclic_get_skins(){
  return array('@default.xml' => 'default','@blue.xml' => 'blue','@orange.xml' => 'orange','@green.xml' => 'green','@simple.xml' => 'simple');
} 




/**
* Get moodle server
*
* @return string                myserver.com:port
*/
function jclic_get_server() {
    global $CFG;

    if (!empty($CFG->wwwroot)) {
        $url = parse_url($CFG->wwwroot);
    }

    if (!empty($url['host'])) {
        $hostname = $url['host'];
    } else if (!empty($_SERVER['SERVER_NAME'])) {
        $hostname = $_SERVER['SERVER_NAME'];
    } else if (!empty($_ENV['SERVER_NAME'])) {
        $hostname = $_ENV['SERVER_NAME'];
    } else if (!empty($_SERVER['HTTP_HOST'])) {
        $hostname = $_SERVER['HTTP_HOST'];
    } else if (!empty($_ENV['HTTP_HOST'])) {
        $hostname = $_ENV['HTTP_HOST'];
    } else {
        notify('Warning: could not find the name of this server!');
        return false;
    }

    if (!empty($url['port'])) {
        $hostname .= ':'.$url['port'];
    } else if (!empty($_SERVER['SERVER_PORT'])) {
        if ($_SERVER['SERVER_PORT'] != 80 && $_SERVER['SERVER_PORT'] != 443) {
            $hostname .= ':'.$_SERVER['SERVER_PORT'];
        }
    }

    return $hostname;
}

/**
* Get moodle path
*
* @return string                /path_to_moodle
*/
function jclic_get_path() {
    global $CFG;

    if (!empty($CFG->wwwroot)) {
        $url = parse_url($CFG->wwwroot);
    }

    return $url['path'];
}

?>
