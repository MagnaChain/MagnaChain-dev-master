<?php

use Xiaoqiang\Mgc\Models\Block;
use Brick\Math\BigInteger;
use Brick\Math\BigDecimal;
use Brick\Math\RoundingMode;

Route::get('/mgcBlock', function() {  //主页的区块信息  取链上所有块中最后5个块

	$block = Db::connection('magnachain')->table('block')->select('height', 'time', 'blockhash', 'blocksize')->orderBy('time', 'desc')->take(5)->get();
	
	$block = $block->toArray();

	$time = 0;
	$day = 0;
	$hour = 0;
	$minute = 0;

	$blockhash_array = array();

	for ($i=0; $i < count($block); $i++) { 
		$blockhash_array[$i] = $block[$i]->blockhash;
	}

	$num = Db::connection('magnachain')->table('transaction')->select('version','blockhash')->whereIn('blockhash', $blockhash_array)->get();
	$num = $num->toArray();

	$tempArr = array();

	foreach($num as $key => $value ){

	    $tempArr[$value->blockhash][] = $value->version;

	}

	for ($i=0; $i < count($block); $i++) {

		$block[$i]->num = count($tempArr[$block[$i]->blockhash]);

		$time = time();

		$minus = $time - $block[$i]->time;

		$day = floor($minus/86400);
		$hour = floor(($minus-86400*$day)/3600);
		$minute = floor(($minus-86400*$day-3600*$hour)/60);
		$block[$i]->time = "";

		if ($day <= 0) {

			$block[$i]->time .= 'a minutes ago';

		} else {

			if ($day>0 && $day == 1) {
				$day = strval($day);
				$block[$i]->time .= $day . ' day ';
			} elseif($day>0 && $day>1) {
				$day = strval($day);
				$block[$i]->time .= $day . ' days ';
			}

			if ($hour>0 && $hour == 1) {
				$hour = strval($hour);
				$block[$i]->time .= $hour . ' hour ';
			} elseif($hour>0 && $hour>1) {
				$hour = strval($hour);
				$block[$i]->time .= $hour . ' hours ';
			}

			if ($minute>0) {
				$minute = strval($minute);
				$block[$i]->time .= $minute . ' minutes ago';
			} elseif ($minute=0) {
				$block[$i]->time .= 'a minutes ago';
			}

		}

		$time = 0;
		$day = 0;
		$hour = 0;
		$minute = 0;

	}

	$block = json_encode($block);

	echo $block;

});

Route::get('/mgcTransaction', function() {  //主页的交易信息

	//$currentTimestamp = Input::get('currentTimestamp');
	//$Timestamp31 = ceil($currentTimestamp/1000) - 31; //31秒前的时间戳

	$block = Db::connection('magnachain')->table('transaction')
    ->leftJoin('block', 'transaction.blockhash', '=', 'block.blockhash')
    ->select('txhash')
    ->orderBy('time', 'desc')
    ->take(5)
    ->get();

	$block = $block->toArray();

	$txhash_array = array();

	for ($i=0; $i < count($block); $i++) { 
		$txhash_array[$i] = $block[$i]->txhash;
	}

	$value = Db::connection('magnachain')->table('txout')->select('txhash', 'value')->whereIn('txhash', $txhash_array)->get();
	$value = $value->toArray();

	$tempArr = array();

	foreach($value as $key => $value ){

	    $tempArr[$value->txhash][] = $value->value;

	}

	for ($i=0; $i < count($tempArr); $i++) { 
		
		$block[$i]->out = BigDecimal::of("0");

		for ($j=0; $j < count($tempArr[$block[$i]->txhash]); $j++) { 
			
			$block[$i]->out = $block[$i]->out->plus($tempArr[$block[$i]->txhash][$j]);

		}

		$block[$i]->out = $block[$i]->out->dividedBy('100000000', 2, RoundingMode::HALF_DOWN);
		$block[$i]->out = strval($block[$i]->out);

	}

	$block = json_encode($block);

	echo $block;

});

Route::post('/date_block', function(){

	$year = Input::post('year');
	$month = Input::post('month');
	$day = Input::post('day');

	$day_array = str_split($day, 1);

	if (count($day_array) == 3) {
		
		if ($day_array[0] == '0') {

			$day = $day_array[1].$day_array[2];

		} else {

			header("Location: /404/请正确输入时间信息~");
			exit();

		}

	} elseif (count($day_array) > 3) {

		header("Location: /404/请正确输入时间信息~");
		exit();

	} elseif (strlen($day) == 1) {
		
		$day = '0'.$day;

	}

	$date = $year.'-'.$month.'-'.$day;

	$data = ['more_block'=>'', 'date'=>$date, 'block'=>null];

	if (strlen($day) == 2) {
		$timestamp = strtotime($year.'-'.$month.'-'.$day.'00:00:00');
	} else {
		$timestamp = strtotime($year.'-'.$month.'-0'.$day.'00:00:00');
	}

	$tomorrow_zero = $timestamp + 86400;

	$block = Db::connection('magnachain')->table('block')->select('height', 'time', 'blockhash', 'bits')->whereBetween('time', [$timestamp, $tomorrow_zero])->orderBy('time', 'esc')->get();
	$block = $block->toArray();

	if ($block == null) {
		echo 0;
		exit();
	}

	$blockhash_array = array();

	for ($i=0; $i < count($block); $i++) { 
		$blockhash_array[$i] = $block[$i]->blockhash;
	}

	$num = Db::connection('magnachain')->table('transaction')->select('version','blockhash')->whereIn('blockhash', $blockhash_array)->get();
	$num = $num->toArray();

	$tempArr = array();

	foreach($num as $key => $value ){

	    $tempArr[$value->blockhash][] = $value->version;

	}

	for ($i=0; $i < count($block); $i++) {

		$block[$i]->num = count($tempArr[$block[$i]->blockhash]);
		$block[$i]->time = date("Y-m-d H:i:s", $block[$i]->time);

	}

	if (count($block)>50) {

		$block = array_slice($block, 0, 50);
		$data['more_block'] = 1;

	}

	$data['block'] = $block;

	$data = json_encode($data);

	echo $data;

});

//大屏幕时使用的搜索路由
Route::get('/search', function(){

	if (Input::get('search')) {
		$search = Input::get('search');
	} elseif (Input::get('search-sm')) {
		$search = Input::get('search-sm');
	}

	if (strlen($search) == 34 && !ctype_digit($search)) {

		//地址
		header("Location: /address/".$search);

	} elseif (strlen($search) == 64) {

		//块或交易
		$block = Db::connection('magnachain')->table('block')->select('height')->where('blockhash', $search)->get();
		$block = $block->toArray();

		$tx = Db::connection('magnachain')->table('transaction')->select('version')->where('txhash', $search)->get();
		$tx = $tx->toArray();

		if ($block) {
			header("Location: /block/".$search);
		} elseif($tx) {
			header("Location: /tx/".$search);
		} else {
			header("Location: /404/请正确输入哈希~");
		}

		
	} elseif (ctype_digit($search)) {

		//块高，纯数字
		$blockhash = Db::connection('magnachain')->table('block')->select('blockhash')->where('height', $search)->get();
		$blockhash = $blockhash->toArray();

		if ($blockhash) {
			$blockhash = $blockhash[0]->blockhash;
			header("Location: /block/".$blockhash);
		} else {
			header("Location: /404/请输入块高~");
		}

	} else {
		//啥也不是
		header("Location: /404");
	}

});

//更多输出
Route::post('/more_output', function(){

	$txhash = Input::post('txhash');

	//存地址和输出
	$out = array();

	$distinct_address = Db::connection('magnachain')->table('txoutpubkey')->select('solution')->where('txhash', $txhash)->distinct()->get();
	$distinct_address = $distinct_address->toArray();

	for($i = 0; $i < count($distinct_address); $i++){
	
		$out[$i]['address'] = $distinct_address[$i]->solution;
		$out[$i]['outNum'] = "0";
		
	}

	//获取交易详情
	$txoutpubkey = Db::connection('magnachain')->table('txoutpubkey')->select('solution', 'txindex')->where('txhash', $txhash)->distinct()->get();
	$txoutpubkey = $txoutpubkey->toArray();

	for($i = 0; $i < count($txoutpubkey); $i++){

		for($j = 0; $j < count($out); $j++){

			if($txoutpubkey[$i]->solution == $out[$j]['address']){

				$txout = Db::connection('magnachain')->table('txout')->select('value')->where('txhash', $txhash)->where('txindex', $txoutpubkey[$i]->txindex)->get();
				$txout = $txout->toArray();

				$out[$j]['outNum'] = BigDecimal::of($out[$j]['outNum'])->plus(BigDecimal::of($txout[0]->value)->dividedBy('100000000', 2, RoundingMode::HALF_DOWN));
				$out[$j]['outNum'] = strval($out[$j]['outNum']);

			}

		}

	}

	//截取5个以后的数据
	$out = array_slice($out, 1);

	$out = json_encode($out);

	echo $out;

});

//更多输入
Route::post('/more_input', function(){
	
	$txhash = Input::post('txhash');

	$in = array();

	$txin = Db::connection('magnachain')->table('txin')->select('outpointhash', 'outpointindex')->where('txhash', '09caa16ebb729286c155733beb4792c0873c947168f7f6061444f6a06ec1193e')->get();
	$txin = $txin->toArray();

	for($i=0; $i < count($txin); $i++){

		$in[$i]['address'] = "";
		$in[$i]['inNum'] = "0";

		$previous_txin = Db::connection('magnachain')->table('txout')->select('value', 'txhash', 'txindex')->where('txhash', $txin[$i]->outpointhash)->where('txindex', $txin[$i]->outpointindex)->get();
		$previous_txin = $previous_txin->toArray();

		$in[$i]['inNum'] = BigDecimal::of($in[$i]['inNum'])->plus(BigDecimal::of($previous_txin[$i]->value)->dividedBy('100000000', 2, RoundingMode::HALF_DOWN));
		$in[$i]['inNum'] = strval($in[$i]['inNum']);

		$previous_address = Db::connection('magnachain')->table('txoutpubkey')->select('solution')->where('txhash', $previous_txin[$i]->txhash)->where('txindex', $previous_txin[$i]->txindex)->get();
		$previous_address = $previous_address->toArray();

		$in[$i]['address'] = $previous_address[0]->solution;

	}

	//截取5个以后的数据
	$in = array_slice($in, 5);

	$in = json_encode($in);

	echo $in;

});

//更多块
Route::post('/more_block', function(){

	$date = Input::post('date');

	$timestamp = strtotime($date.'00:00:00');
	$tomorrow_zero = $timestamp + 86400;

	$block = Db::connection('magnachain')->table('block')->select('height', 'time', 'blockhash', 'bits')->whereBetween('time', [$timestamp, $tomorrow_zero])->orderBy('time', 'esc')->get();
	$block = $block->toArray();

	$blockhash_array = array();

	for ($i=0; $i < count($block); $i++) { 
		$blockhash_array[$i] = $block[$i]->blockhash;
	}

	$num = Db::connection('magnachain')->table('transaction')->select('version','blockhash')->whereIn('blockhash', $blockhash_array)->get();
	$num = $num->toArray();

	$tempArr = array();

	foreach($num as $key => $value ){

	    $tempArr[$value->blockhash][] = $value->version;

	}
	
	for ($i=0; $i < count($block); $i++) {

		$block[$i]->num = count($tempArr[$block[$i]->blockhash]);
		$block[$i]->time = date("Y-m-d H:i:s", $block[$i]->time);

	}

	//$block = array_slice($block, 4);

	$block = json_encode($block);

	echo $block;


});