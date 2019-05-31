<?php 
use Brick\Math\BigInteger;use Brick\Math\BigDecimal;use Brick\Math\RoundingMode;class Cms5cb71a2331c6a390298800_a7646123fe7a989780f08fc6d8dd2bcaClass extends Cms\Classes\PageCode
{



public function onStart(){

	if($this->param('address')){

		self::getBalance($this->param('address'));

		// 1.去outpub表找输出是的哈希
		$whenouttx = Db::connection('magnachain')->table('txoutpubkey')->select('txhash')->where('solution', $this->param('address'))->distinct()->get();
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
		    ->where('txoutpubkey.solution', '=', $this->param('address'))
		    ->get();

		$txinpubkey = $txinpubkey->toArray();

		//将输入、输出时的哈希合并，得到所有有关该地址的交易哈希
		$all_tx = array_merge($whenouttx, $txinpubkey);

		for($l = 0; $l < count($all_tx); $l++){

			$all_tx[$l] = self::object_to_array($all_tx[$l]);

		}

		$all_tx = self::assoc_unique($all_tx, 'txhash');

		$this['address_transaction'] = count($all_tx);

		if (count($all_tx) > 20) {
			
			$this['pagination'] = ceil(count($all_tx)/20);

			$all_tx = array_slice($all_tx, 0, 20);

		}

		$all_tx = self::getInfo($all_tx);

		$all_tx = self::arraySort($all_tx, 'time', $sort = SORT_DESC);

		$this['records'] = $all_tx;

	} else {

		header("Location: /404/请正确输入地址信息~");

	}

}
public function getInfo($all_txhash) {

	$tx_array = array();  //与该地址有关的哈希
	$records = array();  //该地址所有交易记录
	$index = 0;  //记录的脚标
	$in_index = 0;  //输入记录的脚标

	$address = $this->param('address');		//地址	

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

			$this['reward'] = true;

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

			$in_address = self::assoc_unique($in, 'address');

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
public function getBalance($address) {

	$address_allOut = BigDecimal::of('0'); //存该地址的总输入和总输出
	$address_allIn = BigDecimal::of('0'); 	

	//连接查询所有输出
	$allOut = Db::connection('magnachain')->table('txout')
	    ->join('txoutpubkey', function ($join) {
	        $join->on('txoutpubkey.txindex', '=', 'txout.txindex')->on('txoutpubkey.txhash', '=', 'txout.txhash');
	    })
	    ->select('txout.value', 'txoutpubkey.solution', 'txoutpubkey.txhash', 'txoutpubkey.txindex')
	    ->where('txoutpubkey.solution', '=', $this->param('address'))
	    ->get();

	$allOut = $allOut->toArray();

	for ($i=0; $i < count($allOut); $i++) { 
		
		$address_allOut = $address_allOut->plus($allOut[$i]->value);

	}

	//连接查询所有输入
	$allIn = Db::connection('magnachain')->table('txin')
	    ->join('txoutpubkey', function ($join) {
	        $join->on('txin.outpointindex', '=', 'txoutpubkey.txindex')->on('txin.outpointhash', '=', 'txoutpubkey.txhash');
	    })
	    ->join('txout', function ($join) {
	        $join->on('txin.outpointindex', '=', 'txout.txindex')->on('txin.outpointhash', '=', 'txout.txhash');
	    })
	    ->select('txout.value')
	    ->where('txoutpubkey.solution', '=', $this->param('address'))
	    ->get();

	$allIn = $allIn->toArray();

	for ($i=0; $i < count($allIn); $i++) { 
		
		$address_allIn = $address_allIn->plus($allIn[$i]->value);

	}


	$address_allIn = $address_allIn->dividedBy('100000000', 2, RoundingMode::HALF_DOWN);
	$address_allOut = $address_allOut->dividedBy('100000000', 2, RoundingMode::HALF_DOWN);

	$address_balance = floatval(strval($address_allOut)) - floatval(strval($address_allIn));

	$address_balance = sprintf("%.2f", $address_balance);;

	$address_allIn = strval($address_allIn);
	$address_allOut = strval($address_allOut);

	$this['address_allIn'] = $address_allIn;
	$this['address_allOut'] = $address_allOut;
	$this['address_balance'] = $address_balance;

}
public function arraySort($array, $keys, $sort = SORT_DESC) {

    $keysValue = [];

    foreach ($array as $k => $v) {

        $keysValue[$k] = $v[$keys];

    }

    array_multisort($keysValue, $sort, $array);

    return $array;
}
public function assoc_unique($arr, $key) {

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
public function object_to_array($obj) {

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
}
