title = "address"
url = "/address/:address?"
layout = "mgc"
is_hidden = 0

[localePicker]
forceUrl = 0
==
<?php

use Brick\Math\BigInteger;
use Brick\Math\BigDecimal;
use Brick\Math\RoundingMode;

function onStart(){

	if($this->param('address')){

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

		$all_tx = array_slice($all_tx, 0, 15);

		$all_tx = self::getInfo($all_tx);

		$all_tx = self::arraySort($all_tx, 'time', $sort = SORT_DESC);

		$this['records'] = $all_tx;

	} else {

		header("Location: /404/请正确输入地址信息~");

	}

}

function getInfo($all_txhash) {

	$tx_array = array();  //与该地址有关的哈希
	$records = array();  //该地址所有交易记录
	$index = 0;  //记录的脚标
	$in_index = 0;  //输入记录的脚标

	$address = $this->param('address');		//地址	

	$address_allIn = BigDecimal::of('0'); 	//存该地址的总输入和总输出
	$address_allOut = BigDecimal::of('0');

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

					if ($txoutpubkey[$k]->solution == $address) {
						
						$address_allOut = $address_allOut->plus($txoutpubkey[$k]->value);

					}
	
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
						
						if ($in[$p]['address'] == $address) {
						
							$address_allIn = $address_allIn->plus($in[$p]['inNum']);

						}

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

	$address_allIn = $address_allIn->dividedBy('100000000', 2, RoundingMode::HALF_DOWN);
	$address_allOut = $address_allOut->dividedBy('100000000', 2, RoundingMode::HALF_DOWN);

	$address_balance = floatval(strval($address_allOut)) - floatval(strval($address_allIn));

	$address_balance = sprintf("%.2f", $address_balance);;

	$address_allIn = strval($address_allIn);
	$address_allOut = strval($address_allOut);

	$this['address_allIn'] = $address_allIn;
	$this['address_allOut'] = $address_allOut;
	$this['address_balance'] = $address_balance;
	$this['address_transaction'] = count($records);

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

?>
==
<script type="text/javascript" src="/themes/magnachain/assets/js/qrcode.min.js"></script>
<div class="container content">

	<div class="row block-detail">
		
		<div class="col-md-9">
			
			<table class="table">
				<span class="ng-scope" style="font-size: 24px; float: left; margin-top: -20px; margin-bottom: 20px;">{{ '地址详情'|_ }}</span>

				<tbody class="tbody">

				     <tr>
				        <td><span style="float: left; font-weight: bold;">{{ '地址'|_ }}</span></td>
				        <td><span class="address" style="float: right;">{{ this.param.address }}</span></td>
				     </tr>

					 <tr>
				        <td><span style="float: left; font-weight: bold;">{{ '总收入'|_ }}</span></td>
				        <td><span style="float: right;">{{ address_allOut }} MGC</span></td>
				     </tr>

				     <tr>
				        <td><span style="float: left; font-weight: bold;">{{ '总支出'|_ }}</span></td>
				        <td><span style="float: right;">{{ address_allIn }} MGC</span></td>
				     </tr>

				     <tr>
				        <td><span style="float: left; font-weight: bold;">{{ '余额'|_ }}</span></td>
				        <td><span style="float: right;">{{ address_balance }} MGC</span></td>
				     </tr>

				     <tr>
				        <td><span style="float: left; font-weight: bold;">{{ '所有交易'|_ }}</span></td>
				        <td><span style="float: right;">{{ address_transaction }}</span></td>
				     </tr>

				</tbody>

			</table>

		</div>

		<div class="col-md-1" style="text-align: center;">
			<div id="qrcode" style="margin-top: 10px;"></div>
		</div>

	</div>

	<h1 style="font-size: 24px; text-align: left; margin-bottom: 20px; margin-top: 40px;">{{ '交易记录'|_ }}</h1>
		
		{% for record in records %}
		<div class="box row line-mid ng-scope" style="margin-top: 20px; border: 1px solid #ebebeb;">
			
			<a class="hidden-xs hidden-sm" href="/tx/{{ record.txhash }}" style="float: left; margin-top: 14px; margin-bottom: 14px; margin-left: 40px;">
				<span class="ellipsis">{{ record.txhash }}</span>
			</a>
			
			<a class="hidden-md hidden-lg" href="/tx/{{ record.txhash }}" style="float: left; margin-top: 14px; margin-bottom: 14px; margin-left: 20px; overflow: hidden; text-overflow:ellipsis; white-space: nowrap; width: 200px;">
				<span class="ellipsis">{{ record.txhash }}</span>
			</a>
			
			<span class="hidden-xs hidden-sm" style="float: right; margin-top: 14px; margin-bottom: 14px; margin-right: 60px;">{{ '完成时间'|_ }} {{ record.time }}</span>
		
			<span class="hidden-md hidden-lg" style="float: right; margin-top: 14px; margin-bottom: 14px; margin-right: 20px;">{{ '完成时间'|_ }} {{ record.time }}</span>

			<div style="margin-top: 49px; height: 1px; background-color: #ebebeb;"></div>
		
			<div class="row">
				{% if record.in %}

				<div class="col-md-5 col-xs-12 hidden-xs hidden-sm" style="margin-top: 20px; margin-left: 40px; float: left;">

				<div id="input_div_{{ record.txhash }}">
					{% for txin in record.in %}

					<div class="panel panel-default" id="input" style="background-color: #ebebeb; padding-top: 12px; padding-left: 12px;"><a href="/address/{{ txin.address }}" style="float: left;">{{ txin.address }}</a><p>{{ txin.inNum }} MGC</p></div>

					{% endfor %}
				</div>

				{% if record.more_in %}

				<button class="btn btn-default btn-md" id="btn_input_{{ record.txhash }}" style="float: left;" status="1">显示更多</button>

				{% endif %}

				</div>
		
				<div class="col-md-5 col-xs-12 hidden-md hidden-lg" style="margin-top: 20px;">

					<div id="input_div_{{ record.txhash }}">
						{% for txin in record.in %}

						<div class="panel panel-default" id="input" style="background-color: #ebebeb; padding-top: 12px; padding-left: 12px;"><a href="/address/{{ txin.address }}" style="float: left;">{{ txin.address }}</a><p>{{ txin.inNum }} MGC</p></div>

						{% endfor %}
					</div> 

					{% if record.more_in %}

					<button class="btn btn-default btn-md" id="btn_input_{{ record.txhash }}" style="float: left;" data-status="1">显示更多</button>

					{% endif %}

				</div>

				{% else %}

				<div class="col-md-5 col-xs-12 hidden-xs hidden-sm" style="margin-top: 20px; margin-left: 40px;">

					<div class="panel panel-default" style=" background-color: #ebebeb; padding-top: 12px; padding-left: 12px;"><p>{{ '没有交易输入'|_ }}</p></div>

				</div>

				<div class="col-md-5 col-xs-12 hidden-md hidden-lg" style="margin-top: 20px;">

					<div class="panel panel-default" style=" background-color: #ebebeb; padding-top: 12px; padding-left: 12px;"><p>{{ '没有交易输入'|_ }}</p></div>

				</div>

				{% endif %}

				<div class="col-md-1 col-xs-12" style="text-align: center; margin-top: 10px;">

					<div class="hidden-xs hidden-sm"><span style="font-size: 34px; color: #ebebeb;">＞</span></div>

					<div class="hidden-md hidden-lg"><span style="font-size: 34px; color: #ebebeb;">∨</span></div>

				</div>

				<div class="col-md-5 col-xs-12"  style="margin-top: 20px;">

					<div id="output_div_{{ record.txhash }}">
						{% for out in record.out %}

						<div class="panel panel-default" style="background-color: #ebebeb; padding-top: 12px; padding-left: 12px;"><a href="/address/{{ out.address }}" style="float: left;">{{ out.address }}</a><p>{{ out.outNum }} MGC</p></div>

						{% endfor %}
					</div>

					{% if record.more_output %}

					<button class="btn btn-default btn-md" id="btn_output_{{ record.txhash }}" style="float: left;">显示更多</button>

					{% endif %}

				</div>
					
			</div>

			<div style="border-top: 1px solid #ebebeb; margin-top: 10px;">
				{% if record.reward %}
					<div>
						<button type="button" class="btn btn-default btn-sm" disabled="disabled" style="float: left; margin-left: 10px; margin-bottom: 10px; margin-top: 10px;">{{ '矿工费'|_ }}： {{ record.reward }} MGC</button>
					</div>
				{% endif %}
				<div>
					<button type="button" class="btn btn-default btn-sm" disabled="disabled" style="float: right; margin-right: 10px; margin-bottom: 10px; margin-top: 10px;">{{ record.allOut }}MGC</button>
				</div>
			</div>
		
		</div>
		{% endfor %}

	</div>

</div>

<script>
	
	var address = $('.address').text();

	// 设置参数方式
	var qrcode = new QRCode('qrcode', {
	  text: address,
	  width: 200,
	  height: 200,
	  colorDark : '#000000',
	  colorLight : '#ffffff',
	  correctLevel : QRCode.CorrectLevel.H
	});

	// 使用 API
	qrcode.clear();
	qrcode.makeCode(address);

	/*$('#btn-input').click(function(){

		var text = $('#btn-input').text();

		if(text == "显示更多"){
			$("div[id^='input_'").css('display', '').attr('id', 'input_show');
			$('#btn-input').text('显示更少');
		} else {
			$("div[id^='input_'").css('display', 'none').attr('id', 'input_hidden');
			$('#btn-input').text('显示更多');
		}

	});*/

	$("button[id^='btn_input_'").bind("click", function(event){

		event.stopImmediatePropagation();

		var txhash = event.target.id;
		var txhash = txhash.substring(10);

		var input_div = '#input_div_' + txhash;

		var text = $('#btn_input_'+txhash).text();

		if (text == "显示更多") {

			$.post("/more_input", {txhash:txhash}, function (data, status) {

				var data = JSON.parse(data);
				
				$('#btn_input_'+txhash).text('显示更少');

				for(var i = 0; i < data.length; i++){

					$(input_div).append("<div class='panel panel-default' id='more_input_" + txhash + "' style='background-color: #ebebeb; padding-top: 12px; padding-left: 12px;'><a href='/address/" + data[i]['address'] + "' style='float: left;'>" + data[i]['address'] + "</a><p>" + data[i]['inNum'] + " MGC</p></div>");

				}
		   		
			});

		} else {

			$("div[id^='more_input_'").css('display', 'none').attr('id', 'input_hidden');
			$('#btn_input_'+txhash).text('显示更多');

		}
		
	});

	$("button[id^='btn_output_'").bind("click", function(event){

		event.stopImmediatePropagation();

		var txhash = event.target.id;
		var txhash = txhash.substring(11);

		var output_div = '#output_div_' + txhash;

		var text = $('#btn_output_'+txhash).text();

		if (text == "显示更多") {

			$.post("/more_output", {txhash:txhash}, function (data, status) {

				var data = JSON.parse(data);

				$('#btn_output_'+txhash).text('显示更少');

				for(var i = 0; i < data.length; i++){

					console.log(data[i]['address']);

					$(output_div).append("<div class='panel panel-default' id='more_output_" + txhash + "' style='background-color: #ebebeb; padding-top: 12px; padding-left: 12px;'><a href='/address/" + data[i]['address'] + "' style='float: left;'>" + data[i]['address'] + "</a><p>" + data[i]['outNum'] + " MGC</p></div>");

				}
		   		

			});

		} else {

			$("div[id^='more_output_'").css('display', 'none').attr('id', 'input_hidden');
			$('#btn_output_'+txhash).text('显示更多');

		}
		
	});


</script>