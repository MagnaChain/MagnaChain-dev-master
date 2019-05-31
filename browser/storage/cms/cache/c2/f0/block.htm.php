<?php 
use Brick\Math\BigInteger;use Brick\Math\BigDecimal;use Brick\Math\RoundingMode;class Cms5cb43a1e5d31a720795559_dec004c3e71f2b0e9944805744cd5bbaClass extends Cms\Classes\PageCode
{
public function onStart() {

	
	
	

	$str_blockhash = $this->param('blockhash');
	$str_blockhash = strval($str_blockhash);

	if (ctype_digit($str_blockhash)) {

		//块高，纯数字
		$blockhash = Db::connection('magnachain')->table('block')->select('blockhash')->where('height', $str_blockhash)->get();
		$blockhash = $blockhash->toArray();

		if ($blockhash) {
			header("Location: /block/".$blockhash[0]->blockhash);
		} else {
			header("Location: /404/请正确输入块高~");
		}

		exit();

	} elseif ($str_blockhash == false) {

		//块高，纯数字
		$blockhash = Db::connection('magnachain')->table('block')->select('blockhash')->where('height', "0")->get();
		$blockhash = $blockhash->toArray();

		if ($blockhash) {
			header("Location: /block/".$blockhash[0]->blockhash);
		} else {
			header("Location: /404/请正确输入块高~");
		}

		exit();

	}
	
	if($this->param('blockhash')){

		$records = array();  //该地址所有交易记录
		$index = 0;  //记录的脚标
		$in_index = 0;  //输入记录的脚标
		
		$in = array();  //存地址和输入、输出
		$out = array();

		$block = Db::connection('magnachain')->table('block')->select('blockhash', 'height', 'time', 'hashmerkleroot', 'nonce', 'bits', 'version', 'blocksize')->where('blockhash', $this->param('blockhash'))->get();
		$block = $block->toArray();

		if(empty($block)){

			header("Location: /404/找不到块~");		//404处理
			exit();

		}

		$difficulty = self::getDifficutly($block[0]->bits);

		$difficulty = strval($difficulty);

		$block[0]->difficulty = BigDecimal::of($difficulty);

		$num = Db::connection('magnachain')->table('transaction')->select('version')->where('blockhash', $block[0]->blockhash)->get();
		$num = $num->toArray();

		$block[0]->num = count($num);
		$block[0]->time = date("Y-m-d H:i:s", $block[0]->time);
		
		$this['block'] = $block;

		//查找块下所有交易哈希
		$tx = Db::connection('magnachain')->table('transaction')->select('txhash')->where('blockhash', $this->param('blockhash'))->get();
		$tx = $tx->toArray();
		
		for($i=0; $i < count($tx); $i++){  //for循环取出所有该交易的收入

			$tx_array[$index] = $tx[$i]->txhash;
			$records[$index]['txhash'] = $tx[$i]->txhash;
			$records[$index]['allOut'] = BigDecimal::of('0');
			$records[$index]['allIn'] = BigDecimal::of('0');

			//用交易哈希去找块哈希
			$block = Db::connection('magnachain')->table('transaction')->select('blockhash')->where('txhash', $tx[$i]->txhash)->get();
			$block = $block->toArray();

			//查询交易时间
			$txtime = Db::connection('magnachain')->table('block')->select('time')->where('blockhash', $block[0]->blockhash)->get();
			$txtime = $txtime->toArray();
			
			$records[$index]['time'] = date("Y-m-d H:i:s", $txtime[0]->time);

			//查询该交易哈希下的输出地址，去重
			$distinct_address = Db::connection('magnachain')->table('txoutpubkey')->select('solution')->where('txhash', $tx[$i]->txhash)->distinct()->get();
			$distinct_address = $distinct_address->toArray();

			for($j = 0; $j < count($distinct_address); $j++){

				$out[$j]['address'] = $distinct_address[$j]->solution;
				$out[$j]['outNum'] = '0';
			
			}

			//获取交易详情
			$txoutpubkey = Db::connection('magnachain')->table('txoutpubkey')
		    ->join('txout', function ($join) {
		        $join->on('txoutpubkey.txindex', '=', 'txout.txindex')->on('txoutpubkey.txhash', '=', 'txout.txhash');
		    })
		    ->select('txoutpubkey.solution', 'txoutpubkey.txindex', 'txout.value')
		    ->where('txoutpubkey.txhash', '=', $tx[$i]->txhash)
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

			$txin = Db::connection('magnachain')->table('txin')->select('outpointhash', 'outpointindex')->where('txhash', $tx[$i]->txhash)->get();
			$txin = $txin->toArray();

			if(empty($txin)){
				
				$this['reward'] = true;

				$blockreward = $records[$index]['allOut']->dividedBy('100000000', 2, RoundingMode::HALF_DOWN);
				$blockreward = strval($blockreward);

				$this['blockreward'] = $blockreward;

				$records[$index]['blockreward'] = true;

				$records[$index]['allIn'] = null;
				$records[$index]['in'] = null;
				$in_address = null;


			} else {	

				for($y=0; $y < count($txin); $y++){  //将所有输入存入in
					
					$in[$in_index]['address'] = "";
					$in[$in_index]['inNum'] = "0";

					$previous_txin = Db::connection('magnachain')->table('txout')->select('value', 'txhash', 'txindex')->where('txhash', $txin[$y]->outpointhash)->where('txindex', $txin[$y]->outpointindex)->get();
					$previous_txin = $previous_txin->toArray();

					$in[$in_index]['inNum'] = $previous_txin[$y]->value;
			
					$previous_address = Db::connection('magnachain')->table('txoutpubkey')->select('solution')->where('txhash', $txin[$y]->outpointhash)->where('txindex', $txin[$y]->outpointindex)->get();
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
		
		$this['records'] = $records;

	} else {

		header("Location: /404/请正确输入块哈希~");

	}

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
public function getDifficutly($nBits){

	$nShift = ($nBits >> 24) & 0xff;

    $dDiff = (double)0x0000ffff / (double)($nBits & 0x00ffffff);

    while ($nShift < 29) {
        $dDiff *= 256.0;
        $nShift++;
    }

    while ($nShift > 29) {
        $dDiff /= 256.0;
        $nShift--;
    }

    return $dDiff;

}
}
