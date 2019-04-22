<?php

use Brick\Math\BigInteger;
use Brick\Math\BigDecimal;
use Brick\Math\RoundingMode;

function onStart() {

	if ($this->param('address')) {

		$address_allOut = BigDecimal::of('0'); //存该地址的总输入和总输出
		$address_allIn = BigDecimal::of('0'); 	
	
		//连接查询所有输出
		$allOut = Db::connection('magnachain')->table('txout')
		    ->join('txoutpubkey', function ($join) {
		        $join->on('txoutpubkey.txindex', '=', 'txout.txindex')->on('txoutpubkey.txhash', '=', 'txout.txhash');
		    })
		    ->select('txout.value')
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

		$address_allIn = strval($address_allIn);
		$address_allOut = strval($address_allOut);

		var_dump($address_allIn);
		var_dump($address_allOut);

	}

}