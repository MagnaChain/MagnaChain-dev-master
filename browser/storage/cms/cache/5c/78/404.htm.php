<?php 
class Cms5cb42d5b48110946907407_65650793dab87961ff6d23f1ebc2208eClass extends Cms\Classes\PageCode
{
public function onStart() {
	
	if($this->param('error')){

		$this['error'] = $this->param('error');

	}

}
}
