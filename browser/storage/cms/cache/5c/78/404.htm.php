<?php 
class Cms5cb6d9b8c8ffc487806177_4e1df41818193297ec6501266e8d24f1Class extends Cms\Classes\PageCode
{
public function onStart() {

	if($this->param('error')){

		if($this->activeLocaleName == 'English'){

			switch ($this->param('error')) {

				case '请正确输入时间信息~':
					$this['error'] = 'Please enter the time information correctly~';
				break;

				case '请正确输入哈希~':
					$this['error'] = 'Please enter hash correctly~';
				break;

				case '该区块不存在~':
					$this['error'] = 'This block does not exist~';
				break;

				case '找不到地址~':
					$this['error'] = 'Can`t find address~';
				break;

				case '请正确输入地址信息~':
					$this['error'] = 'Please enter address correctly~';
				break;

				case '请正确输入块高~':
				# code...
				break;

				case '找不到块~':
					$this['error'] = 'Please enter the height of block correctly~';
				break;

				case '请正确输入交易哈希~':
					$this['error'] = 'Please enter the transaction hash correctly~';
				break;

			}

		} else {

			$this['error'] = $this->param('error');

		}

	}

}
}
