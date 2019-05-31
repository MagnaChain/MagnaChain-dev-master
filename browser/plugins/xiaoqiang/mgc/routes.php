<?php

use Xiaoqiang\Mgc\Models\Block;
use Brick\Math\BigInteger;
use Brick\Math\BigDecimal;
use Brick\Math\RoundingMode;

Route::get('/mgcBlock', function() {  //主页的区块信息  取链上所有块中最后5个块

	$language = Input::get('language');

	$str_day = $language=='Chinese'?'天':' day ';
	$str_days = $language=='Chinese'?'天':' days ';
	$str_hour = $language=='Chinese'?'时':' hour ';
	$str_hours = $language=='Chinese'?'时':' hours ';
	$str_minute = $language=='Chinese'?'1分钟前':'a minutes ago';
	$str_minutes = $language=='Chinese'?'分钟前':' minutes ago';

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

			$block[$i]->time .= $str_minute;

		} else {

			if ($day>0 && $day == 1) {
				$day = strval($day);
				$block[$i]->time .= $day . $str_day;
			} elseif($day>0 && $day>1) {
				$day = strval($day);
				$block[$i]->time .= $day . $str_days;
			}

			if ($hour>0 && $hour == 1) {
				$hour = strval($hour);
				$block[$i]->time .= $hour . $str_hour;
			} elseif($hour>0 && $hour>1) {
				$hour = strval($hour);
				$block[$i]->time .= $hour . $str_hours;
			}

			if ($minute>0) {
				$minute = strval($minute);
				$block[$i]->time .= $minute . $str_minutes;
			} elseif ($minute=0) {
				$block[$i]->time .= $str_minute;
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

	} elseif (count($day_array) == 1) {
		
		$day = '0'.$day;

	}

	$date = $year.'-'.$month.'-'.$day;

	$data = ['pagination'=>null, 'total_page'=>null, 'date'=>$date, 'block'=>null];

	if (strlen($day) == 2) {
		$timestamp = strtotime($year.'-'.$month.'-'.$day.'00:00:00');
	} else {
		$timestamp = strtotime($year.'-'.$month.'-0'.$day.'00:00:00');
	}

	$tomorrow_zero = $timestamp + 86400;

	$block = Db::connection('magnachain')->table('block')->select('height', 'time', 'blockhash', 'bits')->whereBetween('time', [$timestamp, $tomorrow_zero])->orderBy('time', 'esc')->take(50)->get();
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

	$count = Db::connection('magnachain')->table('block')->select('height')->whereBetween('time', [$timestamp, $tomorrow_zero])->count();

	$pagination = ceil($count/50);

	$data['block'] = $block;
	$data['pagination'] = $pagination;
	$data['total_page'] = $count;

	$data = json_encode($data);

	echo $data;

});

Route::get('/all_blocks_pagination', function(){

	$page = Input::get('page');
	$total_page = Input::get('total_page');
	$date = Input::get('date');

	if (!ctype_digit($page) || $page == 0) {
		$page = 0;
	} elseif ($page>=1 && $page<$total_page-1) {
		$page = $page-1;
	} elseif ($page>$total_page-1){
		$page = $total_page-1;
	} elseif ($page=$total_page-1 && $total_page>=2) {
		$page = $total_page-1-1;
	}

	$data = ['block'=>null, 'page'=>$page+1];

	$timestamp = strtotime($date.'00:00:00');
	$tomorrow_zero = $timestamp + 86400;

	$block = Db::connection('magnachain')->table('block')->select('height', 'time', 'blockhash', 'blocksize','height')->whereBetween('time', [$timestamp, $tomorrow_zero])->orderBy('time', 'esc')->skip($page*50)->take(50)->get();
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

	$data['block'] = $block;

	echo json_encode($data);

});

//      ===================  address_pagination     start  =========================
Route::get('/address_pagination', function() {

	$page = Input::get('page');
	$address = Input::get('address');
	$page = $page-1;

	$data = ['records'=>null];

	// 1.去outpub表找输出是的哈希
	$whenouttx = Db::connection('magnachain')->table('txoutpubkey')->select('txhash')->where('solution', $address)->distinct()->get();
	$whenouttx = $whenouttx->toArray();

	if(empty($whenouttx)){

		header("Location: /404/找不到地址~");		//404处理
		exit();

	}

	//连接查询所有哈希
	$txinpubkey = Db::connection('magnachain')->table('txoutpubkey')
	    ->join('txout', function ($join) {
	        $join->on('txoutpubkey.txindex', '=', 'txout.txindex')->on('txoutpubkey.txhash', '=', 'txout.txhash');
	    })
	    ->join('txin', function ($join) {
	        $join->on('txoutpubkey.txindex', '=', 'txin.outpointindex')->on('txoutpubkey.txhash', '=', 'txin.outpointhash');
	    })
	    ->select('txin.txhash')
	    ->where('txoutpubkey.solution', '=', $address)
	    ->get();

	$txinpubkey = $txinpubkey->toArray();

	//将输入、输出时的哈希合并，得到所有有关该地址的交易哈希
	$all_tx = array_merge($whenouttx, $txinpubkey);

	for($l = 0; $l < count($all_tx); $l++){

		$all_tx[$l] = object_to_array($all_tx[$l]);

	}

	$all_tx = assoc_unique($all_tx, 'txhash');

	if (count($all_tx) > 20) {

		if ($page <= 0) {
			$all_tx = array_slice($all_tx, 0, 20);
		} else {
			$all_tx = array_slice($all_tx, $page*20-1, 20);
		}

	}

	$all_tx = getInfo($all_tx, $address);

	$all_tx = arraySort($all_tx, 'time', $sort = SORT_DESC);

	$data['records'] = $all_tx;

	echo json_encode($data);

});

function getInfo($all_txhash, $address) {

	$tx_array = array();  //与该地址有关的哈希
	$records = array();  //该地址所有交易记录
	$index = 0;  //记录的脚标
	$in_index = 0;  //输入记录的脚标

	for($i=0; $i < count($all_txhash); $i++){  //for循环取出所有该地址的收入

		$in = array();  //存地址和输入、输出
		$out = array();

		$tx_array[$index] = $all_txhash[$i]['txhash'];
		$records[$index]['txhash'] = $all_txhash[$i]['txhash'];
		$records[$index]['allOut'] = BigDecimal::of('0');
		$records[$index]['allIn'] = BigDecimal::of('0');

		//用交易哈希去找块哈希
		$block = Db::connection('magnachain')->table('transaction')->select('blockhash')->where('txhash', $all_txhash[$i]['txhash'])->get();
		$block = $block->toArray();

		//查询交易时间
		$txtime = Db::connection('magnachain')->table('block')->select('time')->where('blockhash', $block[0]->blockhash)->get();
		$txtime = $txtime->toArray();
		
		$records[$index]['time'] = date("Y-m-d H:i:s", $txtime[0]->time);

		//查询该交易哈希下的地址，去重
		$distinct_address = Db::connection('magnachain')->table('txoutpubkey')->select('solution')->where('txhash', $all_txhash[$i]['txhash'])->distinct()->get();
		$distinct_address = $distinct_address->toArray();

		for($j = 0; $j < count($distinct_address); $j++){
		
			$out[$j]['address'] = $distinct_address[$j]->solution;
			$out[$j]['outNum'] = '0';

		}

		$txoutpubkey = Db::connection('magnachain')->table('txoutpubkey')
		    ->join('txout', function ($join) {
		        $join->on('txoutpubkey.txindex', '=', 'txout.txindex')->on('txoutpubkey.txhash', '=', 'txout.txhash');
		    })
		    ->select('txoutpubkey.solution', 'txoutpubkey.txindex', 'txout.value')
		    ->where('txoutpubkey.txhash', '=', $all_txhash[$i]['txhash'])
		    ->get();

		$txoutpubkey = $txoutpubkey->toArray();
		
		for($k = 0; $k < count($txoutpubkey); $k++){

			for($x = 0; $x < count($out); $x++){

				if($txoutpubkey[$k]->solution == $out[$x]['address']){

					$out[$x]['outNum'] = BigDecimal::of($out[$x]['outNum'])->plus(BigDecimal::of($txoutpubkey[$k]->value)->dividedBy('100000000', 2, RoundingMode::HALF_DOWN));
					$out[$x]['outNum'] = strval($out[$x]['outNum']);

					$records[$index]['allOut'] = $records[$index]['allOut']->plus($txoutpubkey[$k]->value);
	
				}

			}

		}

		$txin = Db::connection('magnachain')->table('txin')->select('outpointhash', 'outpointindex')->where('txhash', $all_txhash[$i]['txhash'])->get();
		$txin = $txin->toArray();

		if(empty($txin)){

			$records[$index]['in'] = null; 
			$in_address = null;

			$records[$index]['allIn'] = null;

		} else {
		
			for($y=0; $y < count($txin); $y++){

				$in[$in_index]['address'] = "";
				$in[$in_index]['inNum'] = "0";

				$previous_txin = Db::connection('magnachain')->table('txout')->select('value', 'txhash', 'txindex')->where('txhash', $txin[$y]->outpointhash)->where('txindex', $txin[$y]->outpointindex)->get();
				$previous_txin = $previous_txin->toArray();

				$in[$in_index]['inNum'] = $previous_txin[$y]->value;

				$previous_address = Db::connection('magnachain')->table('txoutpubkey')->select('solution')->where('txhash', $previous_txin[$y]->txhash)->where('txindex', $previous_txin[$y]->txindex)->get();
				$previous_address = $previous_address->toArray();

				$in[$in_index]['address'] = $previous_address[0]->solution;

				$in_index++;

			}

			$in_index = 0;

			$in_address = array();  //存放所有in地址、余额,无重

			$in_address = assoc_unique($in, 'address');

			for ($p=0; $p < count($in); $p++) {

				for ($q=0; $q < count($in_address); $q++) {
					
					if($in[$p]['address'] == $in_address[$q]['address']){

						$in_address[$q]['inNum'] = BigDecimal::of($in_address[$q]['inNum'])->plus(BigDecimal::of($in[$p]['inNum'])->dividedBy('100000000', 2, RoundingMode::HALF_DOWN));
						$in_address[$q]['inNum'] = strval($in_address[$q]['inNum']);

						$records[$index]['allIn'] = $records[$index]['allIn']->plus($in[$p]['inNum']);

					}

				}

			}
	
		}

		if (count($out) > 5) {

			$out = array_slice($out, 0, 5);
			$records[$index]['more_output'] = true;
			
		}

		if (count($in) > 5) {

			$in = array_slice($in, 0, 5);
			$records[$index]['more_input'] = true;
			
		}

		if ($records[$index]['allIn'] != null) {

			$records[$index]['allOut'] = $records[$index]['allOut']->dividedBy('100000000', 2, RoundingMode::HALF_DOWN);
			$records[$index]['allIn'] = $records[$index]['allIn']->dividedBy('100000000', 2, RoundingMode::HALF_DOWN);

			$records[$index]['reward'] = $records[$index]['allIn']->minus($records[$index]['allOut']);

			$records[$index]['reward'] = BigDecimal::of($records[$index]['reward'])->dividedBy('1', 8, RoundingMode::HALF_DOWN);
			$records[$index]['reward'] = strval($records[$index]['reward']);

			$records[$index]['allOut'] = strval($records[$index]['allOut']);
			$records[$index]['allIn'] = strval($records[$index]['allIn']);

			$records[$index]['out'] = $out;  //把输出存在记录中
			$records[$index]['in'] = $in_address;  //把输入存在记录中

		} else {

			$records[$index]['allOut'] = $records[$index]['allOut']->dividedBy('100000000', 2, RoundingMode::HALF_DOWN);

			$records[$index]['allOut'] = strval($records[$index]['allOut']);

			$records[$index]['out'] = $out;  //把输出存在记录中
			$records[$index]['in'] = $in_address;  //把输入存在记录中

		}

		$index++;

	}

	return $records;

}

function arraySort($array, $keys, $sort = SORT_DESC) {

    $keysValue = [];

    foreach ($array as $k => $v) {

        $keysValue[$k] = $v[$keys];

    }

    array_multisort($keysValue, $sort, $array);

    return $array;
}

function assoc_unique($arr, $key) {

	$tmp_arr = array();

	foreach ($arr as $k => $v) {

		if (in_array($v[$key], $tmp_arr)) {//搜索$v[$key]是否在$tmp_arr数组中存在，若存在返回true

			unset($arr[$k]);

		} else {

			$tmp_arr[] = $v[$key];

		}

	}

	sort($arr); //sort函数对数组进行排序

	for ($i=0; $i < count($arr); $i++) { 

		$arr[$i]['inNum'] = "0";

	}

	return $arr;

}

function object_to_array($obj) {

    $obj = (array)$obj;

    foreach ($obj as $k => $v) {
        if (gettype($v) == 'resource') {
            return;
        }
        if (gettype($v) == 'object' || gettype($v) == 'array') {
            $obj[$k] = (array)object_to_array($v);
        }
    }
 
    return $obj;
}

//      ===================  address_pagination     end  =========================

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
			header("Location: /404/该区块不存在~");
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