<?php 
use Brick\Math\BigInteger;use Brick\Math\BigDecimal;use Brick\Math\RoundingMode;class Cms5cb43a2a38888729339036_36c9a2cc7300906d79652d573cc0c142Class extends Cms\Classes\PageCode
{



public function onStart() {

	//总输出
	$this['allOut'] = 0;

	if($this->param('txhash')){

		//交易哈希、大小、接收时间、开采时间、所属块、表示块奖励?
		$details = Db::connection('magnachain')->table('transaction')->select('txhash', 'txsize', 'blockhash')->where('txhash', $this->param('txhash'))->get();
		$details = $details->toArray();

		if($details == null){

			header("Location: /404/找不到交易~");		//404处理
			exit();

		}             
		
		$block = Db::connection('magnachain')->table('block')->select('height', 'time')->where('blockhash', $details[0]->blockhash)->get();
		$block = $block->toArray();

		$details[0]->height = $block[0]->height;
		$details[0]->time = date("Y-m-d H:i:s", $block[0]->time);

		$this['details'] = $details;

		//交易完成时间
		$this['time'] = $details[0]->time;

		//存地址和输入、输出
		$in = array();
		$out = array();

		$distinct_address = Db::connection('magnachain')->table('txoutpubkey')->select('solution')->where('txhash', $this->param('txhash'))->distinct()->get();
		$distinct_address = $distinct_address->toArray();

		for($i = 0; $i < count($distinct_address); $i++){
		
			$out[$i]['address'] = $distinct_address[$i]->solution;
			$out[$i]['outNum'] = "0";
			
		}

		$records = array();

		$In = BigDecimal::of('0'); 	//存该交易的输入和输出
		$Out = BigDecimal::of('0');

		$allIn = BigDecimal::of('0');  	//存该交易的总输入和总输出
		$allOut = BigDecimal::of('0');

		//获取交易详情
		$txoutpubkey = Db::connection('magnachain')->table('txoutpubkey')
	    ->join('txout', function ($join) {
	        $join->on('txoutpubkey.txindex', '=', 'txout.txindex')->on('txoutpubkey.txhash', '=', 'txout.txhash');
	    })
	    ->select('txoutpubkey.solution', 'txoutpubkey.txindex', 'txout.value')
	    ->where('txoutpubkey.txhash', '=', $this->param('txhash'))
	    ->get();

		for($i = 0; $i < count($txoutpubkey); $i++){

			for($j = 0; $j < count($out); $j++){

				if($txoutpubkey[$i]->solution == $out[$j]['address']){

					$out[$j]['outNum'] = BigDecimal::of($out[$j]['outNum'])->plus(BigDecimal::of($txoutpubkey[$i]->value)->dividedBy('100000000', 2, RoundingMode::HALF_DOWN));
					$out[$j]['outNum'] = strval($out[$j]['outNum']);

					$allOut = BigDecimal::of($allOut)->plus($txoutpubkey[$i]->value);

				}

			}

		}

		/*//获取交易详情
		$txoutpubkey = Db::connection('magnachain')->table('txoutpubkey')->select('solution', 'txindex')->where('txhash', $this->param('txhash'))->distinct()->get();
		$txoutpubkey = $txoutpubkey->toArray();

		for($i = 0; $i < count($txoutpubkey); $i++){

			for($j = 0; $j < count($out); $j++){

				if($txoutpubkey[$i]->solution == $out[$j]['address']){

					$txout = Db::connection('magnachain')->table('txout')->select('value')->where('txhash', $this->param('txhash'))->where('txindex', $txoutpubkey[$i]->txindex)->get();
					$txout = $txout->toArray();

					$out[$j]['outNum'] = BigDecimal::of($out[$j]['outNum'])->plus(BigDecimal::of($txout[0]->value)->dividedBy('100000000', 2, RoundingMode::HALF_DOWN));
					$out[$j]['outNum'] = strval($out[$j]['outNum']);

					$allOut = BigDecimal::of($allOut)->plus($txout[0]->value);

				}

			}

		}*/

		$txin = Db::connection('magnachain')->table('txin')->select('outpointhash', 'outpointindex')->where('txhash', $this->param('txhash'))->get();
		$txin = $txin->toArray();

		if(empty($txin)){

			$this['reward'] = true;
			$allIn = null;

		} else {

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

				$allIn = BigDecimal::of($allIn)->plus($previous_txin[$i]->value);

			}
	
		}

		if (count($out) > 5) {

			$out = array_slice($out, 0, 5);
			$records[0]['more_output'] = true;
			
		}

		if (count($in) > 5) {

			$in = array_slice($in, 0, 5);
			$records[0]['more_input'] = true;
			
		}

		if ($allIn != null) {
			
			$allIn = $allIn->dividedBy('100000000', 2, RoundingMode::HALF_DOWN);
			$allOut = $allOut->dividedBy('100000000', 2, RoundingMode::HALF_DOWN);

			$allIn = strval($allIn);
			$allOut = strval($allOut);

			$free = floatval($allIn) - floatval($allOut);
			$free = sprintf("%.2f", $free);

			$this['free'] = $free;

		} else {

			$allOut = $allOut->dividedBy('100000000', 2, RoundingMode::HALF_DOWN);
			$allOut = strval($allOut);
		}

		$records[0]['in'] = $in;
		$records[0]['out'] = $out;

		$records[0]['allOut'] = $allOut;
		$records[0]['txhash'] = $this->param('txhash');

		$this['records'] = $records;
		
	} else {

		header("Location: /404/请正确输入交易哈希~");

	}

}
}
