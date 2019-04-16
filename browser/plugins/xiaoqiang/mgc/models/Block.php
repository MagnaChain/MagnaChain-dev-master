<?php namespace Xiaoqiang\Mgc\Models;


/**
 * Model
 */
class Block
{
    use \October\Rain\Database\Traits\Validation;
    
    /*
     * Disable timestamps by default.
     * Remove this line if timestamps are defined in the database table.
     */
    public $timestamps = false;

    /**
     * @var string The database table used by the model.
     */
    public $table = 'block';

    //public $connection = Db::connection('mgc')->table('block');

    public $connection = null;

    /**
     * @var array Validation rules
     */
    public $rules = [
    ];

    function __construct(){

    	$this->connection = Db::connection('mgc');

    }

    function getBlock(){

    	$block = $connection->where('height', '=', '141')->get();

		$block = json_encode($block);

		return $block;

    }
    
}
