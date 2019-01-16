<?php
header("Content-Type: text/html;charset=utf-8");

error_reporting(0);

$sn = $_GET['sn'];

$time = $_GET['time'];

$token = $_GET['token'];

const  key_secret = "20190114";

$outPut['success'] = ['msg'=>'success','code'=>0,'data'=>''];
$outPut['fail'] = ['msg'=>'fail','code'=>1,'data'=>''];

function returnJson($code,$msg,$data){
    if($code == 0){
        $outPut['success']['msg']=$msg;
        $outPut['success']['data']=$data;
        $outPut['success']['code']=0;
        echo json_encode($outPut['success'],JSON_UNESCAPED_UNICODE);
        die;
    }else{
        $outPut['fail']['code'] = $code;
        $outPut['fail']['msg']=$msg;
        $outPut['fail']['data']=$data;
        echo json_encode($outPut['fail'],JSON_UNESCAPED_UNICODE);
        die;
    }

}

if(empty($sn) || strlen($sn) != 32){
   returnJson(1,'sn error.','');
}

if(empty($time)){
    returnJson(1,'time error.','');
}

if(empty($token)){
    returnJson(1,'token error.','');
}


//检查sn是否合法
$snList = ['f50fdc985e07751a15299b27f7497366'];

if(!in_array($sn,$snList)){
    returnJson(2,'sn error..','');
}

//验证token是否正确
/*echo $sn."\r\n";
echo key_secret."\r\n";
echo $time."\r\n";*/
$new_token = md5($sn.key_secret.$time);

if($new_token != $token){
    returnJson(2,'token error..','');
}else{ //正确返回秘钥
    $key_secret = md5($sn.key_secret);
    returnJson(0,'success',$key_secret);
}




?>