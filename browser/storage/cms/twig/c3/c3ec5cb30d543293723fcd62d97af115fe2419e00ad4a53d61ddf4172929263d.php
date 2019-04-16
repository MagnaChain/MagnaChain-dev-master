<?php

/* D:\MgcBrowser/themes/magnachain/pages/mgc.htm */
class __TwigTemplate_193530d11e9e6907d78890731f018115055a42e097a5eeace30c208d8d65c4d8 extends Twig_Template
{
    private $source;

    public function __construct(Twig_Environment $env)
    {
        parent::__construct($env);

        $this->source = $this->getSourceContext();

        $this->parent = false;

        $this->blocks = [
        ];
    }

    protected function doDisplay(array $context, array $blocks = [])
    {
        // line 1
        echo "<div class=\"container content\">
\t
\t<div class=\"block\">
\t\t
\t\t<span class=\"ng-scope\" style=\"font-size: 24px; float: left;\">";
        // line 5
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["最新区块"]);
        echo "</span>
\t\t
\t\t<a href=\"/all-blocks\" class=\"btn btn-default see-all-blocks\" translate style=\"visibility: hidden;\" ><span class=\"ng-scope\">";
        // line 7
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["查看所有区块"]);
        echo "/span></a>

\t\t\t<div class=\"table-responsive\">

\t\t\t\t<table class=\"table table-striped\">

\t\t\t\t  <thead>
\t\t\t\t     <th class=\"\">";
        // line 14
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["高度"]);
        echo "</th>
\t\t\t\t     <th class=\"\">";
        // line 15
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["时间"]);
        echo "</th>
\t\t\t\t     <th class=\"\">";
        // line 16
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["交易数"]);
        echo "</th>
\t\t\t\t     <th class=\"\">";
        // line 17
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["块大小"]);
        echo "</th>
\t\t\t\t     <th class=\"\">";
        // line 18
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["块哈希"]);
        echo "</th>
\t\t\t\t  </thead>

\t\t\t\t  <tbody class=\"tbody tbody-block\">

\t\t\t\t     <!-- <tr>
\t\t\t\t        <td class=\"\">
\t\t\t\t        \t<a href=\"#\">
\t\t\t\t     \t\t\t\t\t\t\t\t<span class=\"ellipsis ng-binding\">174,663</span>
\t\t\t\t     \t\t\t\t\t\t\t</a></td>
\t\t\t\t        <td class=\"\" style=\"text-align: left;\">a minute ago</td>
\t\t\t\t        <td class=\"\" style=\"text-align: left;\">1</td>
\t\t\t\t        <td class=\"\" style=\"text-align: left;\">312</td>
\t\t\t\t        <td class=\"\" style=\"text-align: left;\">
\t\t\t\t        \t<a href=\"#\" style=\"float: left;\">
\t\t\t\t     \t\t\t\t\t\t\t<span class=\"ellipsis ng-binding\" style=\"white-space:nowrap;\">00000000428d01b71000494f05ee689</span>
\t\t\t\t     \t\t\t\t\t\t\t</a>
\t\t\t\t     \t\t\t\t\t\t</td>
\t\t\t\t     </tr> -->

\t\t\t\t\t <tr>
\t\t\t\t        <td class=\"\" style=\"text-align: left;\" colspan=\"5\">
\t\t\t\t        \t";
        // line 40
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["等待区块中..."]);
        echo "
\t\t\t\t\t\t</td>
\t\t\t\t     </tr>

\t\t\t\t  </tbody>

\t\t\t\t</table>

\t\t\t</div>

\t</div>

\t<div class=\"transaction\">
\t\t
\t\t<span class=\"ng-scope\" style=\"font-size: 24px; float: left;\">";
        // line 54
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["最新交易"]);
        echo "</span>

\t\t<div class=\"table-responsive\">

\t\t\t<table class=\"table table-striped\">

\t\t\t  <thead>
\t\t\t     <th class=\"\">";
        // line 61
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["交易哈希"]);
        echo "</th>
\t\t\t     <th class=\"\">";
        // line 62
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["输出MGC"]);
        echo "</th>
\t\t\t  </thead>

\t\t\t  <tbody class=\"tbody tbody-transaction\">

\t\t\t\t<tr>
\t\t\t        <td class=\"\" style=\"text-align: left;\" colspan=\"5\">
\t\t\t        \t";
        // line 69
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["等待交易中..."]);
        echo "
\t\t\t\t\t</td>
\t\t\t    </tr>

\t\t\t  </tbody>

\t\t\t</table>

\t\t</div>

\t</div>

</div>

<script>
\t
\t\$(function(){

\t\tvar interval = setInterval(function(){

\t\t\t\$.get(\"/mgcBlock\", function (data, status) {

\t\t\t\tif (data != null) {

\t\t\t\t\t//clearInterval(interval);

\t\t\t\t\tvar data = JSON.parse(data)

\t\t\t\t\t\$('.tbody-block').empty();

\t\t\t\t\tfor (var i = 0; i < data.length; i++) {

\t\t\t\t\t\t\$('.tbody-block').append(   \"<tr>\" +
\t\t\t\t\t\t\t\t\t\t\t\t        \"<td class=''>\"+
\t\t\t\t\t\t\t\t\t\t\t\t        \t\"<a href='/block/\"+ data[i]['height'] +\"'>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"<span class='ellipsis ng-binding'>\"+ data[i]['height'] +\"</span>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"</a></td>\"+
\t\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left;'>\"+ data[i]['time'] +\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left;'>\"+ data[i]['num'] +\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left;'>\"+ data[i]['blocksize'] +\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left;'>\"+
\t\t\t\t\t\t\t\t\t\t\t\t        \t\"<a href='/block/\" + data[i]['blockhash'] + \"' style='float: left;'>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"<span class='ellipsis ng-binding' style='white-space:nowrap;'>\"+ data[i]['blockhash'] +\"</span>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"</a>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t\t    \"</tr>\"  );
\t\t\t\t\t}

\t\t\t\t}

\t\t\t});
\t\t
\t\t}, 6000);

\t\t//var timestamp = null;
\t\t//var first

\t\tvar interval2 = setInterval(function(){

\t\t\t//var currentTimestamp = new Date().getTime();

\t\t\t\$.get(\"/mgcTransaction\", function (data, status) {
\t\t\t\t
\t\t\t\tif (data != null) {

\t\t\t\t\tvar data = JSON.parse(data);

\t\t\t\t\t\$('.tbody-transaction').empty();

\t\t\t\t\tfor (var i = 0; i < data.length; i++) {

\t\t\t\t\t\t\$('.tbody-transaction').append(   \"<tr>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t        \"<td class=''>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t        \t\"<a href='/tx/\" + data[i]['txhash'] + \"' style='float: left;'>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"<span class='ellipsis ng-binding'>\"+ data[i]['txhash'] +\"</span>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"</a></td>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='float: left;'>\"+ data[i]['out'] +\" MGC</td>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t     \"</tr>\");
\t\t\t\t\t}

\t\t\t\t}

\t\t\t});
\t\t
\t\t}, 6000);

\t});

</script>";
    }

    public function getTemplateName()
    {
        return "D:\\MgcBrowser/themes/magnachain/pages/mgc.htm";
    }

    public function isTraitable()
    {
        return false;
    }

    public function getDebugInfo()
    {
        return array (  126 => 69,  116 => 62,  112 => 61,  102 => 54,  85 => 40,  60 => 18,  56 => 17,  52 => 16,  48 => 15,  44 => 14,  34 => 7,  29 => 5,  23 => 1,);
    }

    public function getSourceContext()
    {
        return new Twig_Source("<div class=\"container content\">
\t
\t<div class=\"block\">
\t\t
\t\t<span class=\"ng-scope\" style=\"font-size: 24px; float: left;\">{{ '最新区块'|_ }}</span>
\t\t
\t\t<a href=\"/all-blocks\" class=\"btn btn-default see-all-blocks\" translate style=\"visibility: hidden;\" ><span class=\"ng-scope\">{{ '查看所有区块'|_ }}/span></a>

\t\t\t<div class=\"table-responsive\">

\t\t\t\t<table class=\"table table-striped\">

\t\t\t\t  <thead>
\t\t\t\t     <th class=\"\">{{ '高度'|_ }}</th>
\t\t\t\t     <th class=\"\">{{ '时间'|_ }}</th>
\t\t\t\t     <th class=\"\">{{ '交易数'|_ }}</th>
\t\t\t\t     <th class=\"\">{{ '块大小'|_ }}</th>
\t\t\t\t     <th class=\"\">{{ '块哈希'|_ }}</th>
\t\t\t\t  </thead>

\t\t\t\t  <tbody class=\"tbody tbody-block\">

\t\t\t\t     <!-- <tr>
\t\t\t\t        <td class=\"\">
\t\t\t\t        \t<a href=\"#\">
\t\t\t\t     \t\t\t\t\t\t\t\t<span class=\"ellipsis ng-binding\">174,663</span>
\t\t\t\t     \t\t\t\t\t\t\t</a></td>
\t\t\t\t        <td class=\"\" style=\"text-align: left;\">a minute ago</td>
\t\t\t\t        <td class=\"\" style=\"text-align: left;\">1</td>
\t\t\t\t        <td class=\"\" style=\"text-align: left;\">312</td>
\t\t\t\t        <td class=\"\" style=\"text-align: left;\">
\t\t\t\t        \t<a href=\"#\" style=\"float: left;\">
\t\t\t\t     \t\t\t\t\t\t\t<span class=\"ellipsis ng-binding\" style=\"white-space:nowrap;\">00000000428d01b71000494f05ee689</span>
\t\t\t\t     \t\t\t\t\t\t\t</a>
\t\t\t\t     \t\t\t\t\t\t</td>
\t\t\t\t     </tr> -->

\t\t\t\t\t <tr>
\t\t\t\t        <td class=\"\" style=\"text-align: left;\" colspan=\"5\">
\t\t\t\t        \t{{ '等待区块中...'|_ }}
\t\t\t\t\t\t</td>
\t\t\t\t     </tr>

\t\t\t\t  </tbody>

\t\t\t\t</table>

\t\t\t</div>

\t</div>

\t<div class=\"transaction\">
\t\t
\t\t<span class=\"ng-scope\" style=\"font-size: 24px; float: left;\">{{ '最新交易'|_ }}</span>

\t\t<div class=\"table-responsive\">

\t\t\t<table class=\"table table-striped\">

\t\t\t  <thead>
\t\t\t     <th class=\"\">{{ '交易哈希'|_ }}</th>
\t\t\t     <th class=\"\">{{ '输出MGC'|_ }}</th>
\t\t\t  </thead>

\t\t\t  <tbody class=\"tbody tbody-transaction\">

\t\t\t\t<tr>
\t\t\t        <td class=\"\" style=\"text-align: left;\" colspan=\"5\">
\t\t\t        \t{{ '等待交易中...'|_ }}
\t\t\t\t\t</td>
\t\t\t    </tr>

\t\t\t  </tbody>

\t\t\t</table>

\t\t</div>

\t</div>

</div>

<script>
\t
\t\$(function(){

\t\tvar interval = setInterval(function(){

\t\t\t\$.get(\"/mgcBlock\", function (data, status) {

\t\t\t\tif (data != null) {

\t\t\t\t\t//clearInterval(interval);

\t\t\t\t\tvar data = JSON.parse(data)

\t\t\t\t\t\$('.tbody-block').empty();

\t\t\t\t\tfor (var i = 0; i < data.length; i++) {

\t\t\t\t\t\t\$('.tbody-block').append(   \"<tr>\" +
\t\t\t\t\t\t\t\t\t\t\t\t        \"<td class=''>\"+
\t\t\t\t\t\t\t\t\t\t\t\t        \t\"<a href='/block/\"+ data[i]['height'] +\"'>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"<span class='ellipsis ng-binding'>\"+ data[i]['height'] +\"</span>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"</a></td>\"+
\t\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left;'>\"+ data[i]['time'] +\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left;'>\"+ data[i]['num'] +\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left;'>\"+ data[i]['blocksize'] +\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='text-align: left;'>\"+
\t\t\t\t\t\t\t\t\t\t\t\t        \t\"<a href='/block/\" + data[i]['blockhash'] + \"' style='float: left;'>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"<span class='ellipsis ng-binding' style='white-space:nowrap;'>\"+ data[i]['blockhash'] +\"</span>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"</a>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"</td>\"+
\t\t\t\t\t\t\t\t\t\t\t\t    \"</tr>\"  );
\t\t\t\t\t}

\t\t\t\t}

\t\t\t});
\t\t
\t\t}, 6000);

\t\t//var timestamp = null;
\t\t//var first

\t\tvar interval2 = setInterval(function(){

\t\t\t//var currentTimestamp = new Date().getTime();

\t\t\t\$.get(\"/mgcTransaction\", function (data, status) {
\t\t\t\t
\t\t\t\tif (data != null) {

\t\t\t\t\tvar data = JSON.parse(data);

\t\t\t\t\t\$('.tbody-transaction').empty();

\t\t\t\t\tfor (var i = 0; i < data.length; i++) {

\t\t\t\t\t\t\$('.tbody-transaction').append(   \"<tr>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t        \"<td class=''>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t        \t\"<a href='/tx/\" + data[i]['txhash'] + \"' style='float: left;'>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"<span class='ellipsis ng-binding'>\"+ data[i]['txhash'] +\"</span>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"</a></td>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t        \"<td class='' style='float: left;'>\"+ data[i]['out'] +\" MGC</td>\"+
\t\t\t\t\t\t\t\t\t\t\t\t\t     \"</tr>\");
\t\t\t\t\t}

\t\t\t\t}

\t\t\t});
\t\t
\t\t}, 6000);

\t});

</script>", "D:\\MgcBrowser/themes/magnachain/pages/mgc.htm", "");
    }
}
