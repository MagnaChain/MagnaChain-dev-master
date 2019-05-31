<?php 
class Cms5cb944db59056763364942_db13f1c53478380cfa33b9e9467ad8b5Class extends Cms\Classes\PageCode
{
public function onStart() {
	
	if($this->param('date')){

		$date = $this->param('date');                                //现在是写死某一天

		$timestamp = strtotime($date.'00:00:00');
		$tomorrow_zero = $timestamp + 86400;

		if ($timestamp == false) {
			header("Location: /404/请正确输入时间信息~");
			exit();
		}

		$web_date = explode('-', $this->param('date'));

		if (count($web_date) == 2) {
			
			header("Location: /404/请正确输入时间信息~");
			exit();

		} elseif (count($web_date) == 3) {
			
			$day_array = str_split($web_date[2], 1);

			if (count($day_array) == 3) {
		
				if ($day_array[0] == '0') {
	
					$web_date[2] = $day_array[1].$day_array[2];

				} else {

					header("Location: /404/请正确输入时间信息~");
					exit();

				}

			} elseif (count($day_array) > 3) {

				header("Location: /404/请正确输入时间信息~");
				exit();

			}  elseif (count($day_array) == 1) {
		
				$web_date[2] = '0' . $day_array[0];

			}

			$this['year'] = $web_date[0];
			$this['month'] = $web_date[1];
			$this['day'] = $web_date[2];

			$block = Db::connection('magnachain')->table('block')->select('height', 'time', 'blockhash', 'blocksize','height')->whereBetween('time', [$timestamp, $tomorrow_zero])->orderBy('time', 'esc')->take(50)->get();
			$block = $block->toArray();
			
			if ($block == null) {

				$this['nothing'] = "true";
				
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

			if ($count>50) {

				$block = array_slice($block, 0, 50);

				//$this['more_block'] = "true";

			}

			$this['count'] = $count;
			$pagination = ceil($count/50);

			$this['pagination'] = $pagination;

			$this['infos'] = $block;

		}

	}
	
}
}
