<?php

/* D:\MgcBrowser/themes/magnachain/pages/address.htm */
class __TwigTemplate_6de4dbfd45c4bcaea3eb73af3a4ea0c1aaa573a875645fc27bbc223e12b40f3a extends Twig_Template
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
        echo "<script type=\"text/javascript\" src=\"/themes/magnachain/assets/js/qrcode.min.js\"></script>
<div class=\"container content\">

\t<div class=\"row block-detail\">
\t\t
\t\t<div class=\"col-md-9\">
\t\t\t
\t\t\t<table class=\"table\">
\t\t\t\t<span class=\"ng-scope\" style=\"font-size: 24px; float: left; margin-top: -20px; margin-bottom: 20px;\">";
        // line 9
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["地址详情"]);
        echo "</span>

\t\t\t\t<tbody class=\"tbody\">

\t\t\t\t     <tr>
\t\t\t\t        <td><span style=\"float: left; font-weight: bold;\">";
        // line 14
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["地址"]);
        echo "</span></td>
\t\t\t\t        <td><span class=\"address\" style=\"float: right;\">";
        // line 15
        echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, twig_get_attribute($this->env, $this->source, ($context["this"] ?? null), "param", []), "address", []), "html", null, true);
        echo "</span></td>
\t\t\t\t     </tr>

\t\t\t\t\t <tr>
\t\t\t\t        <td><span style=\"float: left; font-weight: bold;\">";
        // line 19
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["总收入"]);
        echo "</span></td>
\t\t\t\t        <td><span style=\"float: right;\">";
        // line 20
        echo twig_escape_filter($this->env, ($context["address_allOut"] ?? null), "html", null, true);
        echo " MGC</span></td>
\t\t\t\t     </tr>

\t\t\t\t     <tr>
\t\t\t\t        <td><span style=\"float: left; font-weight: bold;\">";
        // line 24
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["总支出"]);
        echo "</span></td>
\t\t\t\t        <td><span style=\"float: right;\">";
        // line 25
        echo twig_escape_filter($this->env, ($context["address_allIn"] ?? null), "html", null, true);
        echo " MGC</span></td>
\t\t\t\t     </tr>

\t\t\t\t     <tr>
\t\t\t\t        <td><span style=\"float: left; font-weight: bold;\">";
        // line 29
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["余额"]);
        echo "</span></td>
\t\t\t\t        <td><span style=\"float: right;\">";
        // line 30
        echo twig_escape_filter($this->env, ($context["address_balance"] ?? null), "html", null, true);
        echo " MGC</span></td>
\t\t\t\t     </tr>

\t\t\t\t     <tr>
\t\t\t\t        <td><span style=\"float: left; font-weight: bold;\">";
        // line 34
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["所有交易"]);
        echo "</span></td>
\t\t\t\t        <td><span style=\"float: right;\">";
        // line 35
        echo twig_escape_filter($this->env, ($context["address_transaction"] ?? null), "html", null, true);
        echo "</span></td>
\t\t\t\t     </tr>

\t\t\t\t</tbody>

\t\t\t</table>

\t\t</div>

\t\t<div class=\"col-md-1\" style=\"text-align: center;\">
\t\t\t<div id=\"qrcode\" style=\"margin-top: 10px;\"></div>
\t\t</div>

\t</div>

\t<h1 style=\"font-size: 24px; text-align: left; margin-bottom: 20px; margin-top: 40px;\">";
        // line 50
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["交易记录"]);
        echo "</h1>

\t<div class=\"tx-records\">
\t\t
\t\t";
        // line 54
        $context['_parent'] = $context;
        $context['_seq'] = twig_ensure_traversable(($context["records"] ?? null));
        foreach ($context['_seq'] as $context["_key"] => $context["record"]) {
            // line 55
            echo "\t\t<div class=\"box row line-mid ng-scope\" id=\"tx-records-";
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "txhash", []), "html", null, true);
            echo "\" style=\"margin-top: 20px; border: 1px solid #ebebeb;\">
\t\t\t
\t\t\t<a class=\"hidden-xs hidden-sm\" href=\"/tx/";
            // line 57
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "txhash", []), "html", null, true);
            echo "\" style=\"float: left; margin-top: 14px; margin-bottom: 14px; margin-left: 40px;\">
\t\t\t\t<span class=\"ellipsis\">";
            // line 58
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "txhash", []), "html", null, true);
            echo "</span>
\t\t\t</a>
\t\t\t
\t\t\t<a class=\"hidden-md hidden-lg\" href=\"/tx/";
            // line 61
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "txhash", []), "html", null, true);
            echo "\" style=\"float: left; margin-top: 14px; margin-bottom: 14px; margin-left: 20px; overflow: hidden; text-overflow:ellipsis; white-space: nowrap; width: 200px;\">
\t\t\t\t<span class=\"ellipsis\">";
            // line 62
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "txhash", []), "html", null, true);
            echo "</span>
\t\t\t</a>
\t\t\t
\t\t\t<span class=\"hidden-xs hidden-sm\" style=\"float: right; margin-top: 14px; margin-bottom: 14px; margin-right: 60px;\">";
            // line 65
            echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["完成时间"]);
            echo " ";
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "time", []), "html", null, true);
            echo "</span>
\t\t
\t\t\t<span class=\"hidden-md hidden-lg\" style=\"float: right; margin-top: 14px; margin-bottom: 14px; margin-right: 20px;\">";
            // line 67
            echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["完成时间"]);
            echo " ";
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "time", []), "html", null, true);
            echo "</span>

\t\t\t<div style=\"margin-top: 49px; height: 1px; background-color: #ebebeb;\"></div>
\t\t
\t\t\t<div class=\"row\">
\t\t\t\t";
            // line 72
            if (twig_get_attribute($this->env, $this->source, $context["record"], "in", [])) {
                // line 73
                echo "
\t\t\t\t<div class=\"col-md-5 col-xs-12 hidden-xs hidden-sm\" style=\"margin-top: 20px; margin-left: 40px; float: left;\">

\t\t\t\t<div id=\"input_div_";
                // line 76
                echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "txhash", []), "html", null, true);
                echo "\">
\t\t\t\t\t";
                // line 77
                $context['_parent'] = $context;
                $context['_seq'] = twig_ensure_traversable(twig_get_attribute($this->env, $this->source, $context["record"], "in", []));
                foreach ($context['_seq'] as $context["_key"] => $context["txin"]) {
                    // line 78
                    echo "
\t\t\t\t\t<div class=\"panel panel-default\" id=\"input\" style=\"background-color: #ebebeb; padding-top: 12px; padding-left: 12px;\"><a href=\"/address/";
                    // line 79
                    echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["txin"], "address", []), "html", null, true);
                    echo "\" style=\"float: left;\">";
                    echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["txin"], "address", []), "html", null, true);
                    echo "</a><p>";
                    echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["txin"], "inNum", []), "html", null, true);
                    echo " MGC</p></div>

\t\t\t\t\t";
                }
                $_parent = $context['_parent'];
                unset($context['_seq'], $context['_iterated'], $context['_key'], $context['txin'], $context['_parent'], $context['loop']);
                $context = array_intersect_key($context, $_parent) + $_parent;
                // line 82
                echo "\t\t\t\t</div>

\t\t\t\t";
                // line 84
                if (twig_get_attribute($this->env, $this->source, $context["record"], "more_in", [])) {
                    // line 85
                    echo "
\t\t\t\t<button class=\"btn btn-default btn-md\" id=\"btn_input_";
                    // line 86
                    echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "txhash", []), "html", null, true);
                    echo "\" style=\"float: left;\" status=\"1\">显示更多</button>

\t\t\t\t";
                }
                // line 89
                echo "
\t\t\t\t</div>
\t\t
\t\t\t\t<div class=\"col-md-5 col-xs-12 hidden-md hidden-lg\" style=\"margin-top: 20px;\">

\t\t\t\t\t<div id=\"input_div_";
                // line 94
                echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "txhash", []), "html", null, true);
                echo "\">
\t\t\t\t\t\t";
                // line 95
                $context['_parent'] = $context;
                $context['_seq'] = twig_ensure_traversable(twig_get_attribute($this->env, $this->source, $context["record"], "in", []));
                foreach ($context['_seq'] as $context["_key"] => $context["txin"]) {
                    // line 96
                    echo "
\t\t\t\t\t\t<div class=\"panel panel-default\" id=\"input\" style=\"background-color: #ebebeb; padding-top: 12px; padding-left: 12px;\"><a href=\"/address/";
                    // line 97
                    echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["txin"], "address", []), "html", null, true);
                    echo "\" style=\"float: left;\">";
                    echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["txin"], "address", []), "html", null, true);
                    echo "</a><p>";
                    echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["txin"], "inNum", []), "html", null, true);
                    echo " MGC</p></div>

\t\t\t\t\t\t";
                }
                $_parent = $context['_parent'];
                unset($context['_seq'], $context['_iterated'], $context['_key'], $context['txin'], $context['_parent'], $context['loop']);
                $context = array_intersect_key($context, $_parent) + $_parent;
                // line 100
                echo "\t\t\t\t\t</div> 

\t\t\t\t\t";
                // line 102
                if (twig_get_attribute($this->env, $this->source, $context["record"], "more_in", [])) {
                    // line 103
                    echo "
\t\t\t\t\t<button class=\"btn btn-default btn-md\" id=\"btn_input_";
                    // line 104
                    echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "txhash", []), "html", null, true);
                    echo "\" style=\"float: left;\" data-status=\"1\">显示更多</button>

\t\t\t\t\t";
                }
                // line 107
                echo "
\t\t\t\t</div>

\t\t\t\t";
            } else {
                // line 111
                echo "
\t\t\t\t<div class=\"col-md-5 col-xs-12 hidden-xs hidden-sm\" style=\"margin-top: 20px; margin-left: 40px;\">

\t\t\t\t\t<div class=\"panel panel-default\" style=\" background-color: #ebebeb; padding-top: 12px; padding-left: 12px;\"><p>";
                // line 114
                echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["没有交易输入"]);
                echo "</p></div>

\t\t\t\t</div>

\t\t\t\t<div class=\"col-md-5 col-xs-12 hidden-md hidden-lg\" style=\"margin-top: 20px;\">

\t\t\t\t\t<div class=\"panel panel-default\" style=\" background-color: #ebebeb; padding-top: 12px; padding-left: 12px;\"><p>";
                // line 120
                echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["没有交易输入"]);
                echo "</p></div>

\t\t\t\t</div>

\t\t\t\t";
            }
            // line 125
            echo "
\t\t\t\t<div class=\"col-md-1 col-xs-12\" style=\"text-align: center; margin-top: 10px;\">

\t\t\t\t\t<div class=\"hidden-xs hidden-sm\"><span style=\"font-size: 34px; color: #ebebeb;\">＞</span></div>

\t\t\t\t\t<div class=\"hidden-md hidden-lg\"><span style=\"font-size: 34px; color: #ebebeb;\">∨</span></div>

\t\t\t\t</div>

\t\t\t\t<div class=\"col-md-5 col-xs-12\"  style=\"margin-top: 20px;\">

\t\t\t\t\t<div id=\"output_div_";
            // line 136
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "txhash", []), "html", null, true);
            echo "\">
\t\t\t\t\t\t";
            // line 137
            $context['_parent'] = $context;
            $context['_seq'] = twig_ensure_traversable(twig_get_attribute($this->env, $this->source, $context["record"], "out", []));
            foreach ($context['_seq'] as $context["_key"] => $context["out"]) {
                // line 138
                echo "
\t\t\t\t\t\t<div class=\"panel panel-default\" style=\"background-color: #ebebeb; padding-top: 12px; padding-left: 12px;\"><a href=\"/address/";
                // line 139
                echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["out"], "address", []), "html", null, true);
                echo "\" style=\"float: left;\">";
                echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["out"], "address", []), "html", null, true);
                echo "</a><p>";
                echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["out"], "outNum", []), "html", null, true);
                echo " MGC</p></div>

\t\t\t\t\t\t";
            }
            $_parent = $context['_parent'];
            unset($context['_seq'], $context['_iterated'], $context['_key'], $context['out'], $context['_parent'], $context['loop']);
            $context = array_intersect_key($context, $_parent) + $_parent;
            // line 142
            echo "\t\t\t\t\t</div>

\t\t\t\t\t";
            // line 144
            if (twig_get_attribute($this->env, $this->source, $context["record"], "more_output", [])) {
                // line 145
                echo "
\t\t\t\t\t<button class=\"btn btn-default btn-md\" id=\"btn_output_";
                // line 146
                echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "txhash", []), "html", null, true);
                echo "\" style=\"float: left;\">显示更多</button>

\t\t\t\t\t";
            }
            // line 149
            echo "
\t\t\t\t</div>
\t\t\t\t\t
\t\t\t</div>

\t\t\t<div style=\"border-top: 1px solid #ebebeb; margin-top: 10px;\">
\t\t\t\t";
            // line 155
            if (twig_get_attribute($this->env, $this->source, $context["record"], "reward", [])) {
                // line 156
                echo "\t\t\t\t\t<div>
\t\t\t\t\t\t<button type=\"button\" class=\"btn btn-default btn-sm\" disabled=\"disabled\" style=\"float: left; margin-left: 10px; margin-bottom: 10px; margin-top: 10px;\">";
                // line 157
                echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["矿工费"]);
                echo "： ";
                echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "reward", []), "html", null, true);
                echo " MGC</button>
\t\t\t\t\t</div>
\t\t\t\t";
            }
            // line 160
            echo "\t\t\t\t<div>
\t\t\t\t\t<button type=\"button\" class=\"btn btn-default btn-sm\" disabled=\"disabled\" style=\"float: right; margin-right: 10px; margin-bottom: 10px; margin-top: 10px;\">";
            // line 161
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "allOut", []), "html", null, true);
            echo "MGC</button>
\t\t\t\t</div>
\t\t\t</div>
\t\t
\t\t</div>
\t\t";
        }
        $_parent = $context['_parent'];
        unset($context['_seq'], $context['_iterated'], $context['_key'], $context['record'], $context['_parent'], $context['loop']);
        $context = array_intersect_key($context, $_parent) + $_parent;
        // line 167
        echo "
\t</div>

\t</div>

\t<!-- pagination -->

\t<p id=\"web_page\" page=\"";
        // line 174
        echo twig_escape_filter($this->env, ($context["pagination"] ?? null), "html", null, true);
        echo "\" style=\"visibility: hidden;\"></p>

\t";
        // line 176
        if ((($context["pagination"] ?? null) > 4)) {
            // line 177
            echo "
\t<ul class=\"pagination\" id=\"pagination\">
\t\t<!-- <li><a href=\"javascript:void(0);\" id=\"pagination_previous\" style=\"color: #777777\">&laquo;</a></li> -->
\t\t";
            // line 180
            $context['_parent'] = $context;
            $context['_seq'] = twig_ensure_traversable(range(1, 5));
            foreach ($context['_seq'] as $context["_key"] => $context["i"]) {
                // line 181
                echo "\t\t\t";
                if (($context["i"] == 1)) {
                    // line 182
                    echo "\t\t\t<li><a href=\"javascript:void(0);\" id=\"pagination_1\" page=\"1\" style=\"background-color: #ebebeb; color: #777777\">";
                    echo twig_escape_filter($this->env, $context["i"], "html", null, true);
                    echo "</a></li>
\t\t\t";
                } else {
                    // line 184
                    echo "\t\t\t<li><a href=\"javascript:void(0);\" id=\"pagination_";
                    echo twig_escape_filter($this->env, $context["i"], "html", null, true);
                    echo "\" page=\"";
                    echo twig_escape_filter($this->env, $context["i"], "html", null, true);
                    echo "\" style=\"color: #777777\">";
                    echo twig_escape_filter($this->env, $context["i"], "html", null, true);
                    echo "</a></li>
\t\t\t";
                }
                // line 186
                echo "\t\t";
            }
            $_parent = $context['_parent'];
            unset($context['_seq'], $context['_iterated'], $context['_key'], $context['i'], $context['_parent'], $context['loop']);
            $context = array_intersect_key($context, $_parent) + $_parent;
            // line 187
            echo "\t\t<li><a href=\"javascript:void(0);\" id=\"pagination_next\" page=\"2\" style=\"color: #777777\">&raquo;</a></li>
\t\t<p style=\"display: inline-block;\">&nbsp 共";
            // line 188
            echo twig_escape_filter($this->env, ($context["pagination"] ?? null), "html", null, true);
            echo "页  跳到</p>
\t\t<input type=\"text\" id=\"page-input\" name=\"page-input\" style=\"width: 54px;\">
\t\t<p style=\"display: inline-block;\">页</p>
\t\t<button class=\"btn btn-default\" id=\"confirm-page\" onclick=\"pageInfo();\" style=\"display: inline-block;\">确认</button>
\t</ul>

\t";
        } else {
            // line 195
            echo "
\t<ul class=\"pagination\" id=\"pagination\" style=\"display: none;\">
\t\t<!-- <li><a href=\"javascript:void(0);\" id=\"pagination_previous\" style=\"color: #777777\">&laquo;</a></li> -->
\t\t";
            // line 198
            $context['_parent'] = $context;
            $context['_seq'] = twig_ensure_traversable(range(1, ($context["pagination"] ?? null)));
            foreach ($context['_seq'] as $context["_key"] => $context["i"]) {
                // line 199
                echo "\t\t\t";
                if (($context["i"] == 1)) {
                    // line 200
                    echo "\t\t\t<li><a href=\"javascript:void(0);\" id=\"pagination_1\" page=\"1\" class=\"avtive\" style=\"background-color: #ebebeb; color: #777777\">";
                    echo twig_escape_filter($this->env, $context["i"], "html", null, true);
                    echo "</a></li>
\t\t\t";
                } else {
                    // line 202
                    echo "\t\t\t<li><a href=\"javascript:void(0);\" id=\"pagination_";
                    echo twig_escape_filter($this->env, $context["i"], "html", null, true);
                    echo "\" page=";
                    echo twig_escape_filter($this->env, $context["i"], "html", null, true);
                    echo " style=\"color: #777777\">";
                    echo twig_escape_filter($this->env, $context["i"], "html", null, true);
                    echo "</a></li>
\t\t\t";
                }
                // line 204
                echo "\t\t";
            }
            $_parent = $context['_parent'];
            unset($context['_seq'], $context['_iterated'], $context['_key'], $context['i'], $context['_parent'], $context['loop']);
            $context = array_intersect_key($context, $_parent) + $_parent;
            // line 205
            echo "\t\t<li><a href=\"javascript:void(0);\" id=\"pagination_next\" page=\"2\" style=\"color: #777777\">&raquo;</a></li>
\t\t<p style=\"display: inline-block;\">&nbsp 共";
            // line 206
            echo twig_escape_filter($this->env, ($context["pagination"] ?? null), "html", null, true);
            echo "页  跳到</p>
\t\t<input type=\"text\" id=\"page-input\" name=\"page-input\" style=\"width: 54px;\">
\t\t<p style=\"display: inline-block;\">页</p>
\t\t<button class=\"btn btn-default\" id=\"confirm-page\" onclick=\"pageInfo();\" style=\"display: inline-block;\">确认</button>
\t</ul>

\t";
        }
        // line 213
        echo "
\t<!-- pagination end --> 

</div>

<script>
\t
\tvar address = \$('.address').text();

\t// 设置参数方式
\tvar qrcode = new QRCode('qrcode', {
\t  text: address,
\t  width: 200,
\t  height: 200,
\t  colorDark : '#000000',
\t  colorLight : '#ffffff',
\t  correctLevel : QRCode.CorrectLevel.H
\t});

\t// 使用 API
\tqrcode.clear();
\tqrcode.makeCode(address);

\t/*\$('#btn-input').click(function(){

\t\tvar text = \$('#btn-input').text();

\t\tif(text == \"显示更多\"){
\t\t\t\$(\"div[id^='input_'\").css('display', '').attr('id', 'input_show');
\t\t\t\$('#btn-input').text('显示更少');
\t\t} else {
\t\t\t\$(\"div[id^='input_'\").css('display', 'none').attr('id', 'input_hidden');
\t\t\t\$('#btn-input').text('显示更多');
\t\t}

\t});*/

\t\$(\"button[id^='btn_input_'\").bind(\"click\", function(event){

\t\tevent.stopImmediatePropagation();

\t\tvar txhash = event.target.id;
\t\tvar txhash = txhash.substring(10);

\t\tvar input_div = '#input_div_' + txhash;

\t\tvar text = \$('#btn_input_'+txhash).text();

\t\tif (text == \"显示更多\") {

\t\t\t\$.post(\"/more_input\", {txhash:txhash}, function (data, status) {

\t\t\t\tvar data = JSON.parse(data);
\t\t\t\t
\t\t\t\t\$('#btn_input_'+txhash).text('显示更少');

\t\t\t\tfor(var i = 0; i < data.length; i++){

\t\t\t\t\t\$(input_div).append(\"<div class='panel panel-default' id='more_input_\" + txhash + \"' style='background-color: #ebebeb; padding-top: 12px; padding-left: 12px;'><a href='/address/\" + data[i]['address'] + \"' style='float: left;'>\" + data[i]['address'] + \"</a><p>\" + data[i]['inNum'] + \" MGC</p></div>\");

\t\t\t\t}
\t\t   \t\t
\t\t\t});

\t\t} else {

\t\t\t\$(\"div[id^='more_input_'\").css('display', 'none').attr('id', 'input_hidden');
\t\t\t\$('#btn_input_'+txhash).text('显示更多');

\t\t}
\t\t
\t});

\t\$(\"button[id^='btn_output_'\").bind(\"click\", function(event){

\t\tevent.stopImmediatePropagation();

\t\tvar txhash = event.target.id;
\t\tvar txhash = txhash.substring(11);

\t\tvar output_div = '#output_div_' + txhash;

\t\tvar text = \$('#btn_output_'+txhash).text();

\t\tif (text == \"显示更多\") {

\t\t\t\$.post(\"/more_output\", {txhash:txhash}, function (data, status) {

\t\t\t\tvar data = JSON.parse(data);

\t\t\t\t\$('#btn_output_'+txhash).text('显示更少');

\t\t\t\tfor(var i = 0; i < data.length; i++){

\t\t\t\t\t\$(output_div).append(\"<div class='panel panel-default' id='more_output_\" + txhash + \"' style='background-color: #ebebeb; padding-top: 12px; padding-left: 12px;'><a href='/address/\" + data[i]['address'] + \"' style='float: left;'>\" + data[i]['address'] + \"</a><p>\" + data[i]['outNum'] + \" MGC</p></div>\");

\t\t\t\t}
\t\t   \t\t

\t\t\t});

\t\t} else {

\t\t\t\$(\"div[id^='more_output_'\").css('display', 'none').attr('id', 'input_hidden');
\t\t\t\$('#btn_output_'+txhash).text('显示更多');

\t\t}
\t\t
\t});


\t\$(\"#pagination a\").bind(\"click\", function(event){
\t
\t\tevent.stopImmediatePropagation();

\t\tvar page = \$(this).attr('page');
\t\tvar total_page = \$(\"#web_page\").attr('page');
\t\tvar address = \$(\".address\").text();

\t\tpage = parseInt(page);
\t\ttotal_page = parseInt(total_page);

\t\tif (page <= 0) {
\t\t\tpage = 1;
\t\t} else if (page > total_page) {
\t\t\tpage = total_page;
\t\t}
\t\t
\t\tgetPageInfo(page, total_page, address);
\t\t
\t});

\t\$('#page-input').bind('keydown', function(event){

        if(event.keyCode == \"13\")    
        {

        \tvar page = \$(this).val();
        \tvar total_page = \$(\"#web_page\").attr('page');
        \tvar address = \$(\".address\").text();
         \t
         \tif (page != null) {

         \t\tpage = parseInt(page);
\t\t\t\ttotal_page = parseInt(total_page);

\t\t\t\tif (page <= 0) {
\t\t\t\t\tpage = 1;
\t\t\t\t} else if (page > total_page) {
\t\t\t\t\tpage = total_page;
\t\t\t\t}

         \t\tgetPageInfo(page, total_page, address);

         \t}

        }

    });

    function pageInfo(){

    \tvar page = \$('#page-input').val();
    \tvar total_page = \$(\"#web_page\").attr('page');
    \tvar address = \$(\".address\").text();
     \t
     \tif (page != null) {

     \t\tpage = parseInt(page);
\t\t\ttotal_page = parseInt(total_page);

\t\t\tif (page <= 0) {
\t\t\t\tpage = 1;
\t\t\t} else if (page > total_page) {
\t\t\t\tpage = total_page;
\t\t\t}

     \t\tgetPageInfo(page, total_page, address);

     \t}

    }

    function getPageInfo(page, total_page, address){

    \tpage = parseInt(page);
\t\ttotal_page = parseInt(total_page);

    \t\$.get(\"/address_pagination\", {address:address, page:page}, function (data, status) {

       \t\tif (data != null) {

       \t\t\t\$('.tbody-block').empty();

       \t\t\tvar data = JSON.parse(data);

       \t\t\tvar html = '';

       \t\t\tfor(var i = 0; i < data['records'].length; i++){

       \t\t\t\thtml += \"<div class='box row line-mid ng-scope' id='tx-records-\"+ data['records'][i]['txhash']+ \"' style='margin-top: 20px; border: 1px solid #ebebeb;'>\";

       \t\t\t\thtml += \"<a class='hidden-xs hidden-sm' href='/tx/\"+ data['records'][i]['txhash'] +\"' style='float: left; margin-top: 14px; margin-bottom: 14px; margin-left: 40px;'>\" +
\t\t\t\t\t\t\t\t\"<span class='ellipsis'>\"+ data['records'][i]['txhash'] +\"</span>\" +
\t\t\t\t\t\t\t\"</a>\";

\t\t\t\t\thtml += \"<a class='hidden-md hidden-lg' href='/tx/\"+ data['records'][i]['txhash'] +\"' style='float: left; margin-top: 14px; margin-bottom: 14px; margin-left: 20px; overflow: hidden; text-overflow:ellipsis; white-space: nowrap; width: 200px;'>\" +
\t\t\t\t\t\t\t\t\"<span class='ellipsis'>\"+ data['records'][i]['txhash'] +\"</span>\" +
\t\t\t\t\t\t\t\"</a>\";

\t\t\t\t\thtml += \"<span class='hidden-xs hidden-sm' style='float: right; margin-top: 14px; margin-bottom: 14px; margin-right: 60px;'>";
        // line 423
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["完成时间"]);
        echo " \"+ data['records'][i]['time'] +\"</span>\";

\t\t\t\t\thtml += \"<span class='hidden-md hidden-lg' style='float: right; margin-top: 14px; margin-bottom: 14px; margin-right: 60px;'>";
        // line 425
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["完成时间"]);
        echo " \"+ data['records'][i]['time'] +\"</span>\";

\t\t\t\t\thtml += \"<div style='margin-top: 49px; height: 1px; background-color: #ebebeb;'></div>\";

\t\t\t\t\thtml += \"<div class='row'>\";

\t\t\t\t\tif (data['records'][i]['in'] != null) {

\t\t\t\t\t\thtml += \"<div class='col-md-5 col-xs-12 hidden-xs hidden-sm' style='margin-top: 20px; margin-left: 40px; float: left;'>\";

\t\t\t\t\t\thtml += \"<div id='input_div_\"+ data['records'][i]['txhash'] +\"'>\";

\t\t\t\t\t\tfor(var n = 0; n < data['records'][i]['in'].length; n++){

\t\t\t\t\t\t\thtml += \"<div class='panel panel-default' id='input' style='background-color: #ebebeb; padding-top: 12px; padding-left: 12px;'><a href='/address/\"+ data['records'][i]['in'][n]['address'] +\"' style='float: left;'>\"+ data['records'][i]['in'][n]['address'] +\"</a><p>\"+ data['records'][i]['in'][n]['inNum'] +\"MGC</p></div>\";

\t\t\t\t\t\t}

\t\t\t\t\t\thtml += \"</div>\";

\t\t\t\t\t\tif (data['records'][i]['more_in'] != null) {

\t\t\t\t\t\t\thtml += \"<button class='btn btn-default btn-md' id='btn_input_\"+ data['records'][i]['txhash'] +\"' style='float: left;' data-status='1'>显示更多</button>\";

\t\t\t\t\t\t}

\t\t\t\t\t\thtml += \"</div>\";

\t\t\t\t\t} else {

\t\t\t\t\t\thtml += \"<div class='col-md-5 col-xs-12 hidden-xs hidden-sm' style='margin-top: 20px; margin-left: 40px;'>\" +
\t\t\t\t\t\t\t\t\t\"<div class='panel panel-default' style=' background-color: #ebebeb; padding-top: 12px; padding-left: 12px;'><p>";
        // line 456
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["没有交易输入"]);
        echo "</p></div>\" +
\t\t\t\t\t\t        \"</div>\";

\t\t\t\t\t\thtml += \"<div class='col-md-5 col-xs-12 hidden-md hidden-lg' style='margin-top: 20px;'>\" +
\t\t\t\t\t\t\t\t\t\"<div class='panel panel-default' style=' background-color: #ebebeb; padding-top: 12px; padding-left: 12px;'><p>";
        // line 460
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["没有交易输入"]);
        echo "</p></div>\" +
\t\t\t\t\t\t        \"</div>\";

\t\t\t\t\t}

\t\t\t\t\thtml += \"<div class='col-md-1 col-xs-12' style='text-align: center; margin-top: 10px;'>\" +

\t\t\t\t\t\t\t\t\"<div class='hidden-xs hidden-sm'><span style='font-size: 34px; color: #ebebeb;'>＞</span></div>\" +

\t\t\t\t\t\t\t\t\"<div class='hidden-md hidden-lg'><span style='font-size: 34px; color: #ebebeb;'>∨</span></div>\" +

\t\t\t\t\t\t\t\"</div>\" +

\t\t\t\t\t\t\t\"<div class='col-md-5 col-xs-12'  style='margin-top: 20px;'>\" +

\t\t\t\t\t\t\t\t\"<div id='output_div_\"+ data['records'][i]['txhash'] +\"'>\";

\t\t\t\t\tfor(var j = 0; j < data['records'][i]['out'].length; j++){

\t\t\t\t\t\thtml += \"<div class='panel panel-default' style='background-color: #ebebeb; padding-top: 12px; padding-left: 12px;'><a href='/address/\"+ data['records'][i]['out'][j]['address'] +\"' style='float: left;'>\"+ data['records'][i]['out'][j]['address'] +\"</a><p>\"+ data['records'][i]['out'][j]['outNum'] +\" MGC</p></div>\";

\t\t\t\t\t}

\t\t\t\t\thtml += \"</div>\";

\t\t\t\t\tif (data['records'][i]['more_output'] != null) {

\t\t\t\t\t\thtml += \"<button class='btn btn-default btn-md' id='btn_output_\"+ data['records'][i]['txhash'] +\"' style='float: left;'>显示更多</button>\";

\t\t\t\t\t}

\t\t\t\t\thtml += \"</div>\";

\t\t\t\t\thtml += \"</div>\" +

\t\t\t\t\t\t\t\"<div style='border-top: 1px solid #ebebeb; margin-top: 10px;'>\";

\t\t\t\t\tif (data['records'][i]['reward'] != null) {

\t\t\t\t\t\thtml +=\t\"<div>\" +
\t\t\t\t\t\t\t\t\t\"<button type='button' class='btn btn-default btn-sm' disabled='disabled' style='float: left; margin-left: 10px; margin-bottom: 10px; margin-top: 10px;'>";
        // line 500
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["矿工费"]);
        echo "： \"+ data['records'][i]['reward'] +\" MGC</button>\" +
\t\t\t\t\t\t\t\t\"</div>\";

\t\t\t\t\t}

\t\t\t\t\thtml += \"<div>\" +
\t\t\t\t\t\t\t\t\"<button type='button' class='btn btn-default btn-sm' disabled='disabled' style='float: right; margin-right: 10px; margin-bottom: 10px; margin-top: 10px;'>\"+ data['records'][i]['allOut'] +\"MGC</button>\" +
\t\t\t\t\t\t\t\"</div>\";

\t\t\t\t\thtml += \"</div> </div>\"

       \t\t\t}

       \t\t\t\$(\".tx-records\").empty();

       \t\t\t\$(\".tx-records\").append(html);



       \t\t\tif (total_page <= 5) {

       \t\t\t\t\$(\"#pagination\").find('li').remove();

       \t\t\t\tif (page > 1) {
       \t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_previous' page='\"+  (page-1) +\"' style='color: #777777'>&laquo;</a></li>\");
       \t\t\t\t\tregisterClick(\"#pagination_previous\");
       \t\t\t\t}
\t\t\t\t
       \t\t\t\tfor(var i = 0; i < total_page; i++){

       \t\t\t\t\tif (page == i) {
       \t\t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_\"+ i +\"' page='\"+ i +\"' style='background-color: #ebebeb; color: #777777'>\"+ i +\"</a></li>\");
       \t\t\t\t\t\tregisterClick(\"#pagination_\"+i);
       \t\t\t\t\t} else {
       \t\t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_\"+ i +\"' page='\"+ i +\"' style='color: #777777'>\"+ i +\"</a></li>\");
       \t\t\t\t\t\tregisterClick(\"#pagination_\"+i);
       \t\t\t\t\t}  

       \t\t\t\t}
\t\t\t\t\t
\t\t\t\t\tif (page < total_page) {
       \t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_next' page='\"+ (page+1) +\"' style='color: #777777'>&raquo;</a></li>\");
       \t\t\t\t\tregisterClick(\"#pagination_next\");
       \t\t\t\t}

       \t\t\t} else if (total_page > 5) {

       \t\t\t\tif (total_page-data['page']>=2 && page>2) {  //中间情况

       \t\t\t\t\t\$(\"#pagination\").find('li').remove();

\t       \t\t\t\tif (page > 1) {
\t       \t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_previous' page='\"+ (page-1) +\"' style='color: #777777'>&laquo;</a></li>\");
\t       \t\t\t\t\tregisterClick(\"#pagination_previous\");
\t       \t\t\t\t}
\t\t\t\t\t\t
\t       \t\t\t\tfor(var i = (page-2); i <= (page+2); i++){
\t       \t\t\t\t\t
\t       \t\t\t\t\tif (page == i) {
\t       \t\t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_\"+ i +\"' page='\"+ i +\"' style='background-color: #ebebeb; color: #777777'>\"+ i +\"</a></li>\");
\t       \t\t\t\t\t\tregisterClick(\"#pagination_\"+i);
\t       \t\t\t\t\t} else {
\t       \t\t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_\"+ i +\"' page='\"+ i +\"' style='color: #777777'>\"+ i +\"</a></li>\");
\t       \t\t\t\t\t\tregisterClick(\"#pagination_\"+i);
\t       \t\t\t\t\t}  

\t       \t\t\t\t}
\t\t\t\t\t\t
\t\t\t\t\t\tif (page < total_page) {
\t       \t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_next' page='\"+ (page+1) +\"' style='color: #777777'>&raquo;</a></li>\");
\t       \t\t\t\t\tregisterClick(\"#pagination_next\");
\t       \t\t\t\t}

       \t\t\t\t} else if (page<=2){ //最左情况

       \t\t\t\t\t\$(\"#pagination\").find('li').remove();

\t       \t\t\t\tif (page > 1) {
\t       \t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_previous' page='\"+ (page-1) +\"' style='color: #777777'>&laquo;</a></li>\");
\t       \t\t\t\t\tregisterClick(\"#pagination_previous\");
\t       \t\t\t\t}
\t\t\t\t\t
\t       \t\t\t\tfor(var i = 1; i <= 5; i++){

\t       \t\t\t\t\tif (page == i) {
\t       \t\t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_\"+ i +\"' page='\"+ i +\"' style='background-color: #ebebeb; color: #777777'>\"+ i +\"</a></li>\");
\t       \t\t\t\t\t\tregisterClick(\"#pagination_\"+i);
\t       \t\t\t\t\t} else {
\t       \t\t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_\"+ i +\"' page='\"+ i +\"' style='color: #777777'>\"+ i +\"</a></li>\");
\t       \t\t\t\t\t\tregisterClick(\"#pagination_\"+i);
\t       \t\t\t\t\t}  

\t       \t\t\t\t}
\t\t\t\t\t\t
\t\t\t\t\t\tif (page < total_page) {
\t       \t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_next' page='\"+ (page+1) +\"' style='color: #777777'>&raquo;</a></li>\");
\t       \t\t\t\t\tregisterClick(\"#pagination_next\");
\t       \t\t\t\t}


       \t\t\t\t} else if (total_page-page<2){ //最右情况

       \t\t\t\t\t\$(\"#pagination\").find('li').remove();

\t       \t\t\t\tif (page > 1) {
\t       \t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_previous' page='\"+ (page-1) +\"' style='color: #777777'>&laquo;</a></li>\");
\t       \t\t\t\t\tregisterClick(\"#pagination_previous\");
\t       \t\t\t\t}
\t\t\t\t\t
\t       \t\t\t\tfor(var i = (total_page-4); i <= total_page; i++){

\t       \t\t\t\t\tif (page == i) {
\t       \t\t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_\"+ i +\"' page='\"+ i +\"' style='background-color: #ebebeb; color: #777777'>\"+ i +\"</a></li>\");
\t       \t\t\t\t\t\tregisterClick(\"#pagination_\"+i);
\t       \t\t\t\t\t} else {
\t       \t\t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_\"+ i +\"' page='\"+ i +\"' style='color: #777777'>\"+ i +\"</a></li>\");
\t       \t\t\t\t\t\tregisterClick(\"#pagination_\"+i);
\t       \t\t\t\t\t}  

\t       \t\t\t\t}
\t\t\t\t\t\t
\t\t\t\t\t\tif (page < total_page) {
\t       \t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_next' page='\"+ (page+1) +\"' style='color: #777777'>&raquo;</a></li>\");
\t       \t\t\t\t\tregisterClick(\"#pagination_next\");
\t       \t\t\t\t}

       \t\t\t\t}

       \t\t\t}

       \t\t}

\t\t});

    }

    function registerClick(id) {

    \t\$(id).unbind('click');

    \t\$(id).bind(\"click\", function(event){
\t\t
\t\t\tevent.stopImmediatePropagation();

\t\t\tvar page = \$(this).attr('page');
\t\t\tvar total_page = \$(\"#web_page\").attr('page');
\t\t\tvar address = \$(\".address\").text();

\t\t\tpage = parseInt(page);
\t\t\ttotal_page = parseInt(total_page);

\t\t\tif (page <= 0) {
\t\t\t\tpage = 1;
\t\t\t} else if (page > total_page) {
\t\t\t\tpage = total_page;
\t\t\t}

\t\t\tgetPageInfo(page, total_page, address);
\t\t\t
\t\t});

    }


</script>";
    }

    public function getTemplateName()
    {
        return "D:\\MgcBrowser/themes/magnachain/pages/address.htm";
    }

    public function isTraitable()
    {
        return false;
    }

    public function getDebugInfo()
    {
        return array (  780 => 500,  737 => 460,  730 => 456,  696 => 425,  691 => 423,  479 => 213,  469 => 206,  466 => 205,  460 => 204,  450 => 202,  444 => 200,  441 => 199,  437 => 198,  432 => 195,  422 => 188,  419 => 187,  413 => 186,  403 => 184,  397 => 182,  394 => 181,  390 => 180,  385 => 177,  383 => 176,  378 => 174,  369 => 167,  357 => 161,  354 => 160,  346 => 157,  343 => 156,  341 => 155,  333 => 149,  327 => 146,  324 => 145,  322 => 144,  318 => 142,  305 => 139,  302 => 138,  298 => 137,  294 => 136,  281 => 125,  273 => 120,  264 => 114,  259 => 111,  253 => 107,  247 => 104,  244 => 103,  242 => 102,  238 => 100,  225 => 97,  222 => 96,  218 => 95,  214 => 94,  207 => 89,  201 => 86,  198 => 85,  196 => 84,  192 => 82,  179 => 79,  176 => 78,  172 => 77,  168 => 76,  163 => 73,  161 => 72,  151 => 67,  144 => 65,  138 => 62,  134 => 61,  128 => 58,  124 => 57,  118 => 55,  114 => 54,  107 => 50,  89 => 35,  85 => 34,  78 => 30,  74 => 29,  67 => 25,  63 => 24,  56 => 20,  52 => 19,  45 => 15,  41 => 14,  33 => 9,  23 => 1,);
    }

    public function getSourceContext()
    {
        return new Twig_Source("<script type=\"text/javascript\" src=\"/themes/magnachain/assets/js/qrcode.min.js\"></script>
<div class=\"container content\">

\t<div class=\"row block-detail\">
\t\t
\t\t<div class=\"col-md-9\">
\t\t\t
\t\t\t<table class=\"table\">
\t\t\t\t<span class=\"ng-scope\" style=\"font-size: 24px; float: left; margin-top: -20px; margin-bottom: 20px;\">{{ '地址详情'|_ }}</span>

\t\t\t\t<tbody class=\"tbody\">

\t\t\t\t     <tr>
\t\t\t\t        <td><span style=\"float: left; font-weight: bold;\">{{ '地址'|_ }}</span></td>
\t\t\t\t        <td><span class=\"address\" style=\"float: right;\">{{ this.param.address }}</span></td>
\t\t\t\t     </tr>

\t\t\t\t\t <tr>
\t\t\t\t        <td><span style=\"float: left; font-weight: bold;\">{{ '总收入'|_ }}</span></td>
\t\t\t\t        <td><span style=\"float: right;\">{{ address_allOut }} MGC</span></td>
\t\t\t\t     </tr>

\t\t\t\t     <tr>
\t\t\t\t        <td><span style=\"float: left; font-weight: bold;\">{{ '总支出'|_ }}</span></td>
\t\t\t\t        <td><span style=\"float: right;\">{{ address_allIn }} MGC</span></td>
\t\t\t\t     </tr>

\t\t\t\t     <tr>
\t\t\t\t        <td><span style=\"float: left; font-weight: bold;\">{{ '余额'|_ }}</span></td>
\t\t\t\t        <td><span style=\"float: right;\">{{ address_balance }} MGC</span></td>
\t\t\t\t     </tr>

\t\t\t\t     <tr>
\t\t\t\t        <td><span style=\"float: left; font-weight: bold;\">{{ '所有交易'|_ }}</span></td>
\t\t\t\t        <td><span style=\"float: right;\">{{ address_transaction }}</span></td>
\t\t\t\t     </tr>

\t\t\t\t</tbody>

\t\t\t</table>

\t\t</div>

\t\t<div class=\"col-md-1\" style=\"text-align: center;\">
\t\t\t<div id=\"qrcode\" style=\"margin-top: 10px;\"></div>
\t\t</div>

\t</div>

\t<h1 style=\"font-size: 24px; text-align: left; margin-bottom: 20px; margin-top: 40px;\">{{ '交易记录'|_ }}</h1>

\t<div class=\"tx-records\">
\t\t
\t\t{% for record in records %}
\t\t<div class=\"box row line-mid ng-scope\" id=\"tx-records-{{record.txhash}}\" style=\"margin-top: 20px; border: 1px solid #ebebeb;\">
\t\t\t
\t\t\t<a class=\"hidden-xs hidden-sm\" href=\"/tx/{{ record.txhash }}\" style=\"float: left; margin-top: 14px; margin-bottom: 14px; margin-left: 40px;\">
\t\t\t\t<span class=\"ellipsis\">{{ record.txhash }}</span>
\t\t\t</a>
\t\t\t
\t\t\t<a class=\"hidden-md hidden-lg\" href=\"/tx/{{ record.txhash }}\" style=\"float: left; margin-top: 14px; margin-bottom: 14px; margin-left: 20px; overflow: hidden; text-overflow:ellipsis; white-space: nowrap; width: 200px;\">
\t\t\t\t<span class=\"ellipsis\">{{ record.txhash }}</span>
\t\t\t</a>
\t\t\t
\t\t\t<span class=\"hidden-xs hidden-sm\" style=\"float: right; margin-top: 14px; margin-bottom: 14px; margin-right: 60px;\">{{ '完成时间'|_ }} {{ record.time }}</span>
\t\t
\t\t\t<span class=\"hidden-md hidden-lg\" style=\"float: right; margin-top: 14px; margin-bottom: 14px; margin-right: 20px;\">{{ '完成时间'|_ }} {{ record.time }}</span>

\t\t\t<div style=\"margin-top: 49px; height: 1px; background-color: #ebebeb;\"></div>
\t\t
\t\t\t<div class=\"row\">
\t\t\t\t{% if record.in %}

\t\t\t\t<div class=\"col-md-5 col-xs-12 hidden-xs hidden-sm\" style=\"margin-top: 20px; margin-left: 40px; float: left;\">

\t\t\t\t<div id=\"input_div_{{ record.txhash }}\">
\t\t\t\t\t{% for txin in record.in %}

\t\t\t\t\t<div class=\"panel panel-default\" id=\"input\" style=\"background-color: #ebebeb; padding-top: 12px; padding-left: 12px;\"><a href=\"/address/{{ txin.address }}\" style=\"float: left;\">{{ txin.address }}</a><p>{{ txin.inNum }} MGC</p></div>

\t\t\t\t\t{% endfor %}
\t\t\t\t</div>

\t\t\t\t{% if record.more_in %}

\t\t\t\t<button class=\"btn btn-default btn-md\" id=\"btn_input_{{ record.txhash }}\" style=\"float: left;\" status=\"1\">显示更多</button>

\t\t\t\t{% endif %}

\t\t\t\t</div>
\t\t
\t\t\t\t<div class=\"col-md-5 col-xs-12 hidden-md hidden-lg\" style=\"margin-top: 20px;\">

\t\t\t\t\t<div id=\"input_div_{{ record.txhash }}\">
\t\t\t\t\t\t{% for txin in record.in %}

\t\t\t\t\t\t<div class=\"panel panel-default\" id=\"input\" style=\"background-color: #ebebeb; padding-top: 12px; padding-left: 12px;\"><a href=\"/address/{{ txin.address }}\" style=\"float: left;\">{{ txin.address }}</a><p>{{ txin.inNum }} MGC</p></div>

\t\t\t\t\t\t{% endfor %}
\t\t\t\t\t</div> 

\t\t\t\t\t{% if record.more_in %}

\t\t\t\t\t<button class=\"btn btn-default btn-md\" id=\"btn_input_{{ record.txhash }}\" style=\"float: left;\" data-status=\"1\">显示更多</button>

\t\t\t\t\t{% endif %}

\t\t\t\t</div>

\t\t\t\t{% else %}

\t\t\t\t<div class=\"col-md-5 col-xs-12 hidden-xs hidden-sm\" style=\"margin-top: 20px; margin-left: 40px;\">

\t\t\t\t\t<div class=\"panel panel-default\" style=\" background-color: #ebebeb; padding-top: 12px; padding-left: 12px;\"><p>{{ '没有交易输入'|_ }}</p></div>

\t\t\t\t</div>

\t\t\t\t<div class=\"col-md-5 col-xs-12 hidden-md hidden-lg\" style=\"margin-top: 20px;\">

\t\t\t\t\t<div class=\"panel panel-default\" style=\" background-color: #ebebeb; padding-top: 12px; padding-left: 12px;\"><p>{{ '没有交易输入'|_ }}</p></div>

\t\t\t\t</div>

\t\t\t\t{% endif %}

\t\t\t\t<div class=\"col-md-1 col-xs-12\" style=\"text-align: center; margin-top: 10px;\">

\t\t\t\t\t<div class=\"hidden-xs hidden-sm\"><span style=\"font-size: 34px; color: #ebebeb;\">＞</span></div>

\t\t\t\t\t<div class=\"hidden-md hidden-lg\"><span style=\"font-size: 34px; color: #ebebeb;\">∨</span></div>

\t\t\t\t</div>

\t\t\t\t<div class=\"col-md-5 col-xs-12\"  style=\"margin-top: 20px;\">

\t\t\t\t\t<div id=\"output_div_{{ record.txhash }}\">
\t\t\t\t\t\t{% for out in record.out %}

\t\t\t\t\t\t<div class=\"panel panel-default\" style=\"background-color: #ebebeb; padding-top: 12px; padding-left: 12px;\"><a href=\"/address/{{ out.address }}\" style=\"float: left;\">{{ out.address }}</a><p>{{ out.outNum }} MGC</p></div>

\t\t\t\t\t\t{% endfor %}
\t\t\t\t\t</div>

\t\t\t\t\t{% if record.more_output %}

\t\t\t\t\t<button class=\"btn btn-default btn-md\" id=\"btn_output_{{ record.txhash }}\" style=\"float: left;\">显示更多</button>

\t\t\t\t\t{% endif %}

\t\t\t\t</div>
\t\t\t\t\t
\t\t\t</div>

\t\t\t<div style=\"border-top: 1px solid #ebebeb; margin-top: 10px;\">
\t\t\t\t{% if record.reward %}
\t\t\t\t\t<div>
\t\t\t\t\t\t<button type=\"button\" class=\"btn btn-default btn-sm\" disabled=\"disabled\" style=\"float: left; margin-left: 10px; margin-bottom: 10px; margin-top: 10px;\">{{ '矿工费'|_ }}： {{ record.reward }} MGC</button>
\t\t\t\t\t</div>
\t\t\t\t{% endif %}
\t\t\t\t<div>
\t\t\t\t\t<button type=\"button\" class=\"btn btn-default btn-sm\" disabled=\"disabled\" style=\"float: right; margin-right: 10px; margin-bottom: 10px; margin-top: 10px;\">{{ record.allOut }}MGC</button>
\t\t\t\t</div>
\t\t\t</div>
\t\t
\t\t</div>
\t\t{% endfor %}

\t</div>

\t</div>

\t<!-- pagination -->

\t<p id=\"web_page\" page=\"{{ pagination }}\" style=\"visibility: hidden;\"></p>

\t{% if pagination > 4 %}

\t<ul class=\"pagination\" id=\"pagination\">
\t\t<!-- <li><a href=\"javascript:void(0);\" id=\"pagination_previous\" style=\"color: #777777\">&laquo;</a></li> -->
\t\t{% for i in 1..5 %}
\t\t\t{% if i == 1 %}
\t\t\t<li><a href=\"javascript:void(0);\" id=\"pagination_1\" page=\"1\" style=\"background-color: #ebebeb; color: #777777\">{{ i }}</a></li>
\t\t\t{% else %}
\t\t\t<li><a href=\"javascript:void(0);\" id=\"pagination_{{ i }}\" page=\"{{ i }}\" style=\"color: #777777\">{{ i }}</a></li>
\t\t\t{% endif %}
\t\t{% endfor %}
\t\t<li><a href=\"javascript:void(0);\" id=\"pagination_next\" page=\"2\" style=\"color: #777777\">&raquo;</a></li>
\t\t<p style=\"display: inline-block;\">&nbsp 共{{ pagination }}页  跳到</p>
\t\t<input type=\"text\" id=\"page-input\" name=\"page-input\" style=\"width: 54px;\">
\t\t<p style=\"display: inline-block;\">页</p>
\t\t<button class=\"btn btn-default\" id=\"confirm-page\" onclick=\"pageInfo();\" style=\"display: inline-block;\">确认</button>
\t</ul>

\t{% else %}

\t<ul class=\"pagination\" id=\"pagination\" style=\"display: none;\">
\t\t<!-- <li><a href=\"javascript:void(0);\" id=\"pagination_previous\" style=\"color: #777777\">&laquo;</a></li> -->
\t\t{% for i in 1..pagination %}
\t\t\t{% if i == 1 %}
\t\t\t<li><a href=\"javascript:void(0);\" id=\"pagination_1\" page=\"1\" class=\"avtive\" style=\"background-color: #ebebeb; color: #777777\">{{ i }}</a></li>
\t\t\t{% else %}
\t\t\t<li><a href=\"javascript:void(0);\" id=\"pagination_{{ i }}\" page={{ i }} style=\"color: #777777\">{{ i }}</a></li>
\t\t\t{% endif %}
\t\t{% endfor %}
\t\t<li><a href=\"javascript:void(0);\" id=\"pagination_next\" page=\"2\" style=\"color: #777777\">&raquo;</a></li>
\t\t<p style=\"display: inline-block;\">&nbsp 共{{ pagination }}页  跳到</p>
\t\t<input type=\"text\" id=\"page-input\" name=\"page-input\" style=\"width: 54px;\">
\t\t<p style=\"display: inline-block;\">页</p>
\t\t<button class=\"btn btn-default\" id=\"confirm-page\" onclick=\"pageInfo();\" style=\"display: inline-block;\">确认</button>
\t</ul>

\t{% endif %}

\t<!-- pagination end --> 

</div>

<script>
\t
\tvar address = \$('.address').text();

\t// 设置参数方式
\tvar qrcode = new QRCode('qrcode', {
\t  text: address,
\t  width: 200,
\t  height: 200,
\t  colorDark : '#000000',
\t  colorLight : '#ffffff',
\t  correctLevel : QRCode.CorrectLevel.H
\t});

\t// 使用 API
\tqrcode.clear();
\tqrcode.makeCode(address);

\t/*\$('#btn-input').click(function(){

\t\tvar text = \$('#btn-input').text();

\t\tif(text == \"显示更多\"){
\t\t\t\$(\"div[id^='input_'\").css('display', '').attr('id', 'input_show');
\t\t\t\$('#btn-input').text('显示更少');
\t\t} else {
\t\t\t\$(\"div[id^='input_'\").css('display', 'none').attr('id', 'input_hidden');
\t\t\t\$('#btn-input').text('显示更多');
\t\t}

\t});*/

\t\$(\"button[id^='btn_input_'\").bind(\"click\", function(event){

\t\tevent.stopImmediatePropagation();

\t\tvar txhash = event.target.id;
\t\tvar txhash = txhash.substring(10);

\t\tvar input_div = '#input_div_' + txhash;

\t\tvar text = \$('#btn_input_'+txhash).text();

\t\tif (text == \"显示更多\") {

\t\t\t\$.post(\"/more_input\", {txhash:txhash}, function (data, status) {

\t\t\t\tvar data = JSON.parse(data);
\t\t\t\t
\t\t\t\t\$('#btn_input_'+txhash).text('显示更少');

\t\t\t\tfor(var i = 0; i < data.length; i++){

\t\t\t\t\t\$(input_div).append(\"<div class='panel panel-default' id='more_input_\" + txhash + \"' style='background-color: #ebebeb; padding-top: 12px; padding-left: 12px;'><a href='/address/\" + data[i]['address'] + \"' style='float: left;'>\" + data[i]['address'] + \"</a><p>\" + data[i]['inNum'] + \" MGC</p></div>\");

\t\t\t\t}
\t\t   \t\t
\t\t\t});

\t\t} else {

\t\t\t\$(\"div[id^='more_input_'\").css('display', 'none').attr('id', 'input_hidden');
\t\t\t\$('#btn_input_'+txhash).text('显示更多');

\t\t}
\t\t
\t});

\t\$(\"button[id^='btn_output_'\").bind(\"click\", function(event){

\t\tevent.stopImmediatePropagation();

\t\tvar txhash = event.target.id;
\t\tvar txhash = txhash.substring(11);

\t\tvar output_div = '#output_div_' + txhash;

\t\tvar text = \$('#btn_output_'+txhash).text();

\t\tif (text == \"显示更多\") {

\t\t\t\$.post(\"/more_output\", {txhash:txhash}, function (data, status) {

\t\t\t\tvar data = JSON.parse(data);

\t\t\t\t\$('#btn_output_'+txhash).text('显示更少');

\t\t\t\tfor(var i = 0; i < data.length; i++){

\t\t\t\t\t\$(output_div).append(\"<div class='panel panel-default' id='more_output_\" + txhash + \"' style='background-color: #ebebeb; padding-top: 12px; padding-left: 12px;'><a href='/address/\" + data[i]['address'] + \"' style='float: left;'>\" + data[i]['address'] + \"</a><p>\" + data[i]['outNum'] + \" MGC</p></div>\");

\t\t\t\t}
\t\t   \t\t

\t\t\t});

\t\t} else {

\t\t\t\$(\"div[id^='more_output_'\").css('display', 'none').attr('id', 'input_hidden');
\t\t\t\$('#btn_output_'+txhash).text('显示更多');

\t\t}
\t\t
\t});


\t\$(\"#pagination a\").bind(\"click\", function(event){
\t
\t\tevent.stopImmediatePropagation();

\t\tvar page = \$(this).attr('page');
\t\tvar total_page = \$(\"#web_page\").attr('page');
\t\tvar address = \$(\".address\").text();

\t\tpage = parseInt(page);
\t\ttotal_page = parseInt(total_page);

\t\tif (page <= 0) {
\t\t\tpage = 1;
\t\t} else if (page > total_page) {
\t\t\tpage = total_page;
\t\t}
\t\t
\t\tgetPageInfo(page, total_page, address);
\t\t
\t});

\t\$('#page-input').bind('keydown', function(event){

        if(event.keyCode == \"13\")    
        {

        \tvar page = \$(this).val();
        \tvar total_page = \$(\"#web_page\").attr('page');
        \tvar address = \$(\".address\").text();
         \t
         \tif (page != null) {

         \t\tpage = parseInt(page);
\t\t\t\ttotal_page = parseInt(total_page);

\t\t\t\tif (page <= 0) {
\t\t\t\t\tpage = 1;
\t\t\t\t} else if (page > total_page) {
\t\t\t\t\tpage = total_page;
\t\t\t\t}

         \t\tgetPageInfo(page, total_page, address);

         \t}

        }

    });

    function pageInfo(){

    \tvar page = \$('#page-input').val();
    \tvar total_page = \$(\"#web_page\").attr('page');
    \tvar address = \$(\".address\").text();
     \t
     \tif (page != null) {

     \t\tpage = parseInt(page);
\t\t\ttotal_page = parseInt(total_page);

\t\t\tif (page <= 0) {
\t\t\t\tpage = 1;
\t\t\t} else if (page > total_page) {
\t\t\t\tpage = total_page;
\t\t\t}

     \t\tgetPageInfo(page, total_page, address);

     \t}

    }

    function getPageInfo(page, total_page, address){

    \tpage = parseInt(page);
\t\ttotal_page = parseInt(total_page);

    \t\$.get(\"/address_pagination\", {address:address, page:page}, function (data, status) {

       \t\tif (data != null) {

       \t\t\t\$('.tbody-block').empty();

       \t\t\tvar data = JSON.parse(data);

       \t\t\tvar html = '';

       \t\t\tfor(var i = 0; i < data['records'].length; i++){

       \t\t\t\thtml += \"<div class='box row line-mid ng-scope' id='tx-records-\"+ data['records'][i]['txhash']+ \"' style='margin-top: 20px; border: 1px solid #ebebeb;'>\";

       \t\t\t\thtml += \"<a class='hidden-xs hidden-sm' href='/tx/\"+ data['records'][i]['txhash'] +\"' style='float: left; margin-top: 14px; margin-bottom: 14px; margin-left: 40px;'>\" +
\t\t\t\t\t\t\t\t\"<span class='ellipsis'>\"+ data['records'][i]['txhash'] +\"</span>\" +
\t\t\t\t\t\t\t\"</a>\";

\t\t\t\t\thtml += \"<a class='hidden-md hidden-lg' href='/tx/\"+ data['records'][i]['txhash'] +\"' style='float: left; margin-top: 14px; margin-bottom: 14px; margin-left: 20px; overflow: hidden; text-overflow:ellipsis; white-space: nowrap; width: 200px;'>\" +
\t\t\t\t\t\t\t\t\"<span class='ellipsis'>\"+ data['records'][i]['txhash'] +\"</span>\" +
\t\t\t\t\t\t\t\"</a>\";

\t\t\t\t\thtml += \"<span class='hidden-xs hidden-sm' style='float: right; margin-top: 14px; margin-bottom: 14px; margin-right: 60px;'>{{ '完成时间'|_ }} \"+ data['records'][i]['time'] +\"</span>\";

\t\t\t\t\thtml += \"<span class='hidden-md hidden-lg' style='float: right; margin-top: 14px; margin-bottom: 14px; margin-right: 60px;'>{{ '完成时间'|_ }} \"+ data['records'][i]['time'] +\"</span>\";

\t\t\t\t\thtml += \"<div style='margin-top: 49px; height: 1px; background-color: #ebebeb;'></div>\";

\t\t\t\t\thtml += \"<div class='row'>\";

\t\t\t\t\tif (data['records'][i]['in'] != null) {

\t\t\t\t\t\thtml += \"<div class='col-md-5 col-xs-12 hidden-xs hidden-sm' style='margin-top: 20px; margin-left: 40px; float: left;'>\";

\t\t\t\t\t\thtml += \"<div id='input_div_\"+ data['records'][i]['txhash'] +\"'>\";

\t\t\t\t\t\tfor(var n = 0; n < data['records'][i]['in'].length; n++){

\t\t\t\t\t\t\thtml += \"<div class='panel panel-default' id='input' style='background-color: #ebebeb; padding-top: 12px; padding-left: 12px;'><a href='/address/\"+ data['records'][i]['in'][n]['address'] +\"' style='float: left;'>\"+ data['records'][i]['in'][n]['address'] +\"</a><p>\"+ data['records'][i]['in'][n]['inNum'] +\"MGC</p></div>\";

\t\t\t\t\t\t}

\t\t\t\t\t\thtml += \"</div>\";

\t\t\t\t\t\tif (data['records'][i]['more_in'] != null) {

\t\t\t\t\t\t\thtml += \"<button class='btn btn-default btn-md' id='btn_input_\"+ data['records'][i]['txhash'] +\"' style='float: left;' data-status='1'>显示更多</button>\";

\t\t\t\t\t\t}

\t\t\t\t\t\thtml += \"</div>\";

\t\t\t\t\t} else {

\t\t\t\t\t\thtml += \"<div class='col-md-5 col-xs-12 hidden-xs hidden-sm' style='margin-top: 20px; margin-left: 40px;'>\" +
\t\t\t\t\t\t\t\t\t\"<div class='panel panel-default' style=' background-color: #ebebeb; padding-top: 12px; padding-left: 12px;'><p>{{ '没有交易输入'|_ }}</p></div>\" +
\t\t\t\t\t\t        \"</div>\";

\t\t\t\t\t\thtml += \"<div class='col-md-5 col-xs-12 hidden-md hidden-lg' style='margin-top: 20px;'>\" +
\t\t\t\t\t\t\t\t\t\"<div class='panel panel-default' style=' background-color: #ebebeb; padding-top: 12px; padding-left: 12px;'><p>{{ '没有交易输入'|_ }}</p></div>\" +
\t\t\t\t\t\t        \"</div>\";

\t\t\t\t\t}

\t\t\t\t\thtml += \"<div class='col-md-1 col-xs-12' style='text-align: center; margin-top: 10px;'>\" +

\t\t\t\t\t\t\t\t\"<div class='hidden-xs hidden-sm'><span style='font-size: 34px; color: #ebebeb;'>＞</span></div>\" +

\t\t\t\t\t\t\t\t\"<div class='hidden-md hidden-lg'><span style='font-size: 34px; color: #ebebeb;'>∨</span></div>\" +

\t\t\t\t\t\t\t\"</div>\" +

\t\t\t\t\t\t\t\"<div class='col-md-5 col-xs-12'  style='margin-top: 20px;'>\" +

\t\t\t\t\t\t\t\t\"<div id='output_div_\"+ data['records'][i]['txhash'] +\"'>\";

\t\t\t\t\tfor(var j = 0; j < data['records'][i]['out'].length; j++){

\t\t\t\t\t\thtml += \"<div class='panel panel-default' style='background-color: #ebebeb; padding-top: 12px; padding-left: 12px;'><a href='/address/\"+ data['records'][i]['out'][j]['address'] +\"' style='float: left;'>\"+ data['records'][i]['out'][j]['address'] +\"</a><p>\"+ data['records'][i]['out'][j]['outNum'] +\" MGC</p></div>\";

\t\t\t\t\t}

\t\t\t\t\thtml += \"</div>\";

\t\t\t\t\tif (data['records'][i]['more_output'] != null) {

\t\t\t\t\t\thtml += \"<button class='btn btn-default btn-md' id='btn_output_\"+ data['records'][i]['txhash'] +\"' style='float: left;'>显示更多</button>\";

\t\t\t\t\t}

\t\t\t\t\thtml += \"</div>\";

\t\t\t\t\thtml += \"</div>\" +

\t\t\t\t\t\t\t\"<div style='border-top: 1px solid #ebebeb; margin-top: 10px;'>\";

\t\t\t\t\tif (data['records'][i]['reward'] != null) {

\t\t\t\t\t\thtml +=\t\"<div>\" +
\t\t\t\t\t\t\t\t\t\"<button type='button' class='btn btn-default btn-sm' disabled='disabled' style='float: left; margin-left: 10px; margin-bottom: 10px; margin-top: 10px;'>{{ '矿工费'|_ }}： \"+ data['records'][i]['reward'] +\" MGC</button>\" +
\t\t\t\t\t\t\t\t\"</div>\";

\t\t\t\t\t}

\t\t\t\t\thtml += \"<div>\" +
\t\t\t\t\t\t\t\t\"<button type='button' class='btn btn-default btn-sm' disabled='disabled' style='float: right; margin-right: 10px; margin-bottom: 10px; margin-top: 10px;'>\"+ data['records'][i]['allOut'] +\"MGC</button>\" +
\t\t\t\t\t\t\t\"</div>\";

\t\t\t\t\thtml += \"</div> </div>\"

       \t\t\t}

       \t\t\t\$(\".tx-records\").empty();

       \t\t\t\$(\".tx-records\").append(html);



       \t\t\tif (total_page <= 5) {

       \t\t\t\t\$(\"#pagination\").find('li').remove();

       \t\t\t\tif (page > 1) {
       \t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_previous' page='\"+  (page-1) +\"' style='color: #777777'>&laquo;</a></li>\");
       \t\t\t\t\tregisterClick(\"#pagination_previous\");
       \t\t\t\t}
\t\t\t\t
       \t\t\t\tfor(var i = 0; i < total_page; i++){

       \t\t\t\t\tif (page == i) {
       \t\t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_\"+ i +\"' page='\"+ i +\"' style='background-color: #ebebeb; color: #777777'>\"+ i +\"</a></li>\");
       \t\t\t\t\t\tregisterClick(\"#pagination_\"+i);
       \t\t\t\t\t} else {
       \t\t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_\"+ i +\"' page='\"+ i +\"' style='color: #777777'>\"+ i +\"</a></li>\");
       \t\t\t\t\t\tregisterClick(\"#pagination_\"+i);
       \t\t\t\t\t}  

       \t\t\t\t}
\t\t\t\t\t
\t\t\t\t\tif (page < total_page) {
       \t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_next' page='\"+ (page+1) +\"' style='color: #777777'>&raquo;</a></li>\");
       \t\t\t\t\tregisterClick(\"#pagination_next\");
       \t\t\t\t}

       \t\t\t} else if (total_page > 5) {

       \t\t\t\tif (total_page-data['page']>=2 && page>2) {  //中间情况

       \t\t\t\t\t\$(\"#pagination\").find('li').remove();

\t       \t\t\t\tif (page > 1) {
\t       \t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_previous' page='\"+ (page-1) +\"' style='color: #777777'>&laquo;</a></li>\");
\t       \t\t\t\t\tregisterClick(\"#pagination_previous\");
\t       \t\t\t\t}
\t\t\t\t\t\t
\t       \t\t\t\tfor(var i = (page-2); i <= (page+2); i++){
\t       \t\t\t\t\t
\t       \t\t\t\t\tif (page == i) {
\t       \t\t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_\"+ i +\"' page='\"+ i +\"' style='background-color: #ebebeb; color: #777777'>\"+ i +\"</a></li>\");
\t       \t\t\t\t\t\tregisterClick(\"#pagination_\"+i);
\t       \t\t\t\t\t} else {
\t       \t\t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_\"+ i +\"' page='\"+ i +\"' style='color: #777777'>\"+ i +\"</a></li>\");
\t       \t\t\t\t\t\tregisterClick(\"#pagination_\"+i);
\t       \t\t\t\t\t}  

\t       \t\t\t\t}
\t\t\t\t\t\t
\t\t\t\t\t\tif (page < total_page) {
\t       \t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_next' page='\"+ (page+1) +\"' style='color: #777777'>&raquo;</a></li>\");
\t       \t\t\t\t\tregisterClick(\"#pagination_next\");
\t       \t\t\t\t}

       \t\t\t\t} else if (page<=2){ //最左情况

       \t\t\t\t\t\$(\"#pagination\").find('li').remove();

\t       \t\t\t\tif (page > 1) {
\t       \t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_previous' page='\"+ (page-1) +\"' style='color: #777777'>&laquo;</a></li>\");
\t       \t\t\t\t\tregisterClick(\"#pagination_previous\");
\t       \t\t\t\t}
\t\t\t\t\t
\t       \t\t\t\tfor(var i = 1; i <= 5; i++){

\t       \t\t\t\t\tif (page == i) {
\t       \t\t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_\"+ i +\"' page='\"+ i +\"' style='background-color: #ebebeb; color: #777777'>\"+ i +\"</a></li>\");
\t       \t\t\t\t\t\tregisterClick(\"#pagination_\"+i);
\t       \t\t\t\t\t} else {
\t       \t\t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_\"+ i +\"' page='\"+ i +\"' style='color: #777777'>\"+ i +\"</a></li>\");
\t       \t\t\t\t\t\tregisterClick(\"#pagination_\"+i);
\t       \t\t\t\t\t}  

\t       \t\t\t\t}
\t\t\t\t\t\t
\t\t\t\t\t\tif (page < total_page) {
\t       \t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_next' page='\"+ (page+1) +\"' style='color: #777777'>&raquo;</a></li>\");
\t       \t\t\t\t\tregisterClick(\"#pagination_next\");
\t       \t\t\t\t}


       \t\t\t\t} else if (total_page-page<2){ //最右情况

       \t\t\t\t\t\$(\"#pagination\").find('li').remove();

\t       \t\t\t\tif (page > 1) {
\t       \t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_previous' page='\"+ (page-1) +\"' style='color: #777777'>&laquo;</a></li>\");
\t       \t\t\t\t\tregisterClick(\"#pagination_previous\");
\t       \t\t\t\t}
\t\t\t\t\t
\t       \t\t\t\tfor(var i = (total_page-4); i <= total_page; i++){

\t       \t\t\t\t\tif (page == i) {
\t       \t\t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_\"+ i +\"' page='\"+ i +\"' style='background-color: #ebebeb; color: #777777'>\"+ i +\"</a></li>\");
\t       \t\t\t\t\t\tregisterClick(\"#pagination_\"+i);
\t       \t\t\t\t\t} else {
\t       \t\t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_\"+ i +\"' page='\"+ i +\"' style='color: #777777'>\"+ i +\"</a></li>\");
\t       \t\t\t\t\t\tregisterClick(\"#pagination_\"+i);
\t       \t\t\t\t\t}  

\t       \t\t\t\t}
\t\t\t\t\t\t
\t\t\t\t\t\tif (page < total_page) {
\t       \t\t\t\t\t\$(\"#pagination\").append(\"<li><a href='javascript:void(0);' id='pagination_next' page='\"+ (page+1) +\"' style='color: #777777'>&raquo;</a></li>\");
\t       \t\t\t\t\tregisterClick(\"#pagination_next\");
\t       \t\t\t\t}

       \t\t\t\t}

       \t\t\t}

       \t\t}

\t\t});

    }

    function registerClick(id) {

    \t\$(id).unbind('click');

    \t\$(id).bind(\"click\", function(event){
\t\t
\t\t\tevent.stopImmediatePropagation();

\t\t\tvar page = \$(this).attr('page');
\t\t\tvar total_page = \$(\"#web_page\").attr('page');
\t\t\tvar address = \$(\".address\").text();

\t\t\tpage = parseInt(page);
\t\t\ttotal_page = parseInt(total_page);

\t\t\tif (page <= 0) {
\t\t\t\tpage = 1;
\t\t\t} else if (page > total_page) {
\t\t\t\tpage = total_page;
\t\t\t}

\t\t\tgetPageInfo(page, total_page, address);
\t\t\t
\t\t});

    }


</script>", "D:\\MgcBrowser/themes/magnachain/pages/address.htm", "");
    }
}
