<html>
<body>

<h2>Scoreboard</h2>
<?php
//Init int vars to 0 and binary vars to off 
$keys = array('guest','home','inning','balls','strikes','outs');
$vals = array_fill_keys($keys,0);
$bin_keys = array('hit','error','bright');
$bin_vals = array_fill_keys($bin_keys,'');

//copy int vals from form
foreach ($_POST[val] as $key => $value) {
  //echo "{$key} => {$value}<br>";
  $vals[$key]=$value;
}

//copy binary vals from form
foreach ($_POST[bin_val] as $key => $value) {
  //echo "bin {$key} => {$value}<br>";
  if ($value == 'on') {
    $bin_vals[$key]='checked';
  }
  else {
    $bin_vals[$key]='';
  } 
}

//Apply +/- button behavior
foreach ($_POST[button] as $key => $value) {
  //echo "{$key} => {$value}";
  if ($value == '-') {
    $vals[$key]--;
    if ($vals[$key] < 0) {
       $vals[$key] = 0;
    }
  }
  else if ($value == '+') {
    $vals[$key]++;
    if ($vals[$key] == 100) {
      $vals[$key] = 0;
    }
  }
}

//Check overflow for balls/strikes/outs
//Simulate FairPlay controller where 3 strikes
//autoincrements outs
if ($vals['balls'] == 4) {
  $vals['balls'] = 0;
}
if ($vals['strikes'] == 3) {
  $vals['strikes'] = 0;
  $vals['outs']++;
}
if ($vals['outs'] == 3) {
  $vals['outs'] = 0;
}

//write to file
$data='';
foreach ($vals as $key => $value) {
  $data = $data . $value . "\r\n";
}
foreach ($bin_vals as $key => $value) {
  $wval = 0;
  if ($value == 'checked') {
    $wval = 1;
  }
  $data = $data . $wval . "\r\n";
}
$ret = file_put_contents('/var/sb/scoreboard.txt', $data, LOCK_EX);
if ($ret == false) {
  echo "Error writing to file<br>";
}
?>
<br>
<form action="index.php" method="post">
Guest: <input type="text" name="val[guest]" size="1" value="<?php echo $vals[guest];?>" maxlength="2">
  <input type="submit" value="-" name="button[guest]">
  <input type="submit" value="+" name="button[guest]"><br>
Home: <input type="text" name="val[home]" size="2" value="<?php echo $vals[home];?>"  maxlength="2">
  <input type="submit" value="-" name="button[home]">
  <input type="submit" value="+" name="button[home]"><br>
Inning: <input type="text" name="val[inning]" size="2" value="<?php echo $vals[inning];?>" maxlength="2">
  <input type="submit" value="-" name="button[inning]">
  <input type="submit" value="+" name="button[inning]"><br>
Balls: <input type="text" name="val[balls]" size="2" value="<?php echo $vals[balls];?>" maxlength="2">
  <input type="submit" value="-" name="button[balls]">
  <input type="submit" value="+" name="button[balls]"><br>
Strikes: <input type="text" name="val[strikes]" size="2" value="<?php echo $vals[strikes];?>" maxlength="2">
  <input type="submit" value="-" name="button[strikes]">
  <input type="submit" value="+" name="button[strikes]"><br>
Outs: <input type="text" name="val[outs]" size="2" value="<?php echo $vals[outs];?>" maxlength="2">
  <input type="submit" value="-" name="button[outs]">
  <input type="submit" value="+" name="button[outs]"><br>
Hit: <input type="checkbox" name="bin_val[hit]" <?php echo $bin_vals[hit];?>> 
Error: <input type="checkbox" name="bin_val[error]" <?php echo $bin_vals[error];?>><br>
Bright: <input type="checkbox" name="bin_val[bright]" <?php echo $bin_vals[bright];?>><br>
  <input type="submit" value="Update">
</form>
</body>
</html>
