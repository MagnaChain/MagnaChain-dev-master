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
\t\t
\t\t";
        // line 52
        $context['_parent'] = $context;
        $context['_seq'] = twig_ensure_traversable(($context["records"] ?? null));
        foreach ($context['_seq'] as $context["_key"] => $context["record"]) {
            // line 53
            echo "\t\t<div class=\"box row line-mid ng-scope\" style=\"margin-top: 20px; border: 1px solid #ebebeb;\">
\t\t\t
\t\t\t<a class=\"hidden-xs hidden-sm\" href=\"/tx/";
            // line 55
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "txhash", []), "html", null, true);
            echo "\" style=\"float: left; margin-top: 14px; margin-bottom: 14px; margin-left: 40px;\">
\t\t\t\t<span class=\"ellipsis\">";
            // line 56
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "txhash", []), "html", null, true);
            echo "</span>
\t\t\t</a>
\t\t\t
\t\t\t<a class=\"hidden-md hidden-lg\" href=\"/tx/";
            // line 59
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "txhash", []), "html", null, true);
            echo "\" style=\"float: left; margin-top: 14px; margin-bottom: 14px; margin-left: 20px; overflow: hidden; text-overflow:ellipsis; white-space: nowrap; width: 200px;\">
\t\t\t\t<span class=\"ellipsis\">";
            // line 60
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "txhash", []), "html", null, true);
            echo "</span>
\t\t\t</a>
\t\t\t
\t\t\t<span class=\"hidden-xs hidden-sm\" style=\"float: right; margin-top: 14px; margin-bottom: 14px; margin-right: 60px;\">";
            // line 63
            echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["完成时间"]);
            echo " ";
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "time", []), "html", null, true);
            echo "</span>
\t\t
\t\t\t<span class=\"hidden-md hidden-lg\" style=\"float: right; margin-top: 14px; margin-bottom: 14px; margin-right: 20px;\">";
            // line 65
            echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["完成时间"]);
            echo " ";
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "time", []), "html", null, true);
            echo "</span>

\t\t\t<div style=\"margin-top: 49px; height: 1px; background-color: #ebebeb;\"></div>
\t\t
\t\t\t<div class=\"row\">
\t\t\t\t";
            // line 70
            if (twig_get_attribute($this->env, $this->source, $context["record"], "in", [])) {
                // line 71
                echo "
\t\t\t\t<div class=\"col-md-5 col-xs-12 hidden-xs hidden-sm\" style=\"margin-top: 20px; margin-left: 40px; float: left;\">

\t\t\t\t<div id=\"input_div_";
                // line 74
                echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "txhash", []), "html", null, true);
                echo "\">
\t\t\t\t\t";
                // line 75
                $context['_parent'] = $context;
                $context['_seq'] = twig_ensure_traversable(twig_get_attribute($this->env, $this->source, $context["record"], "in", []));
                foreach ($context['_seq'] as $context["_key"] => $context["txin"]) {
                    // line 76
                    echo "
\t\t\t\t\t<div class=\"panel panel-default\" id=\"input\" style=\"background-color: #ebebeb; padding-top: 12px; padding-left: 12px;\"><a href=\"/address/";
                    // line 77
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
                // line 80
                echo "\t\t\t\t</div>

\t\t\t\t";
                // line 82
                if (twig_get_attribute($this->env, $this->source, $context["record"], "more_in", [])) {
                    // line 83
                    echo "
\t\t\t\t<button class=\"btn btn-default btn-md\" id=\"btn_input_";
                    // line 84
                    echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "txhash", []), "html", null, true);
                    echo "\" style=\"float: left;\" status=\"1\">显示更多</button>

\t\t\t\t";
                }
                // line 87
                echo "
\t\t\t\t</div>
\t\t
\t\t\t\t<div class=\"col-md-5 col-xs-12 hidden-md hidden-lg\" style=\"margin-top: 20px;\">

\t\t\t\t\t<div id=\"input_div_";
                // line 92
                echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "txhash", []), "html", null, true);
                echo "\">
\t\t\t\t\t\t";
                // line 93
                $context['_parent'] = $context;
                $context['_seq'] = twig_ensure_traversable(twig_get_attribute($this->env, $this->source, $context["record"], "in", []));
                foreach ($context['_seq'] as $context["_key"] => $context["txin"]) {
                    // line 94
                    echo "
\t\t\t\t\t\t<div class=\"panel panel-default\" id=\"input\" style=\"background-color: #ebebeb; padding-top: 12px; padding-left: 12px;\"><a href=\"/address/";
                    // line 95
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
                // line 98
                echo "\t\t\t\t\t</div> 

\t\t\t\t\t";
                // line 100
                if (twig_get_attribute($this->env, $this->source, $context["record"], "more_in", [])) {
                    // line 101
                    echo "
\t\t\t\t\t<button class=\"btn btn-default btn-md\" id=\"btn_input_";
                    // line 102
                    echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "txhash", []), "html", null, true);
                    echo "\" style=\"float: left;\" data-status=\"1\">显示更多</button>

\t\t\t\t\t";
                }
                // line 105
                echo "
\t\t\t\t</div>

\t\t\t\t";
            } else {
                // line 109
                echo "
\t\t\t\t<div class=\"col-md-5 col-xs-12 hidden-xs hidden-sm\" style=\"margin-top: 20px; margin-left: 40px;\">

\t\t\t\t\t<div class=\"panel panel-default\" style=\" background-color: #ebebeb; padding-top: 12px; padding-left: 12px;\"><p>";
                // line 112
                echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["没有交易输入"]);
                echo "</p></div>

\t\t\t\t</div>

\t\t\t\t<div class=\"col-md-5 col-xs-12 hidden-md hidden-lg\" style=\"margin-top: 20px;\">

\t\t\t\t\t<div class=\"panel panel-default\" style=\" background-color: #ebebeb; padding-top: 12px; padding-left: 12px;\"><p>";
                // line 118
                echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["没有交易输入"]);
                echo "</p></div>

\t\t\t\t</div>

\t\t\t\t";
            }
            // line 123
            echo "
\t\t\t\t<div class=\"col-md-1 col-xs-12\" style=\"text-align: center; margin-top: 10px;\">

\t\t\t\t\t<div class=\"hidden-xs hidden-sm\"><span style=\"font-size: 34px; color: #ebebeb;\">＞</span></div>

\t\t\t\t\t<div class=\"hidden-md hidden-lg\"><span style=\"font-size: 34px; color: #ebebeb;\">∨</span></div>

\t\t\t\t</div>

\t\t\t\t<div class=\"col-md-5 col-xs-12\"  style=\"margin-top: 20px;\">

\t\t\t\t\t<div id=\"output_div_";
            // line 134
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "txhash", []), "html", null, true);
            echo "\">
\t\t\t\t\t\t";
            // line 135
            $context['_parent'] = $context;
            $context['_seq'] = twig_ensure_traversable(twig_get_attribute($this->env, $this->source, $context["record"], "out", []));
            foreach ($context['_seq'] as $context["_key"] => $context["out"]) {
                // line 136
                echo "
\t\t\t\t\t\t<div class=\"panel panel-default\" style=\"background-color: #ebebeb; padding-top: 12px; padding-left: 12px;\"><a href=\"/address/";
                // line 137
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
            // line 140
            echo "\t\t\t\t\t</div>

\t\t\t\t\t";
            // line 142
            if (twig_get_attribute($this->env, $this->source, $context["record"], "more_output", [])) {
                // line 143
                echo "
\t\t\t\t\t<button class=\"btn btn-default btn-md\" id=\"btn_output_";
                // line 144
                echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "txhash", []), "html", null, true);
                echo "\" style=\"float: left;\">显示更多</button>

\t\t\t\t\t";
            }
            // line 147
            echo "
\t\t\t\t</div>
\t\t\t\t\t
\t\t\t</div>

\t\t\t<div style=\"border-top: 1px solid #ebebeb; margin-top: 10px;\">
\t\t\t\t";
            // line 153
            if (twig_get_attribute($this->env, $this->source, $context["record"], "reward", [])) {
                // line 154
                echo "\t\t\t\t\t<div>
\t\t\t\t\t\t<button type=\"button\" class=\"btn btn-default btn-sm\" disabled=\"disabled\" style=\"float: left; margin-left: 10px; margin-bottom: 10px; margin-top: 10px;\">";
                // line 155
                echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["矿工费"]);
                echo "： ";
                echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "reward", []), "html", null, true);
                echo " MGC</button>
\t\t\t\t\t</div>
\t\t\t\t";
            }
            // line 158
            echo "\t\t\t\t<div>
\t\t\t\t\t<button type=\"button\" class=\"btn btn-default btn-sm\" disabled=\"disabled\" style=\"float: right; margin-right: 10px; margin-bottom: 10px; margin-top: 10px;\">";
            // line 159
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
        // line 165
        echo "
\t</div>

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

\t\t\t\t\tconsole.log(data[i]['address']);

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
        return array (  365 => 165,  353 => 159,  350 => 158,  342 => 155,  339 => 154,  337 => 153,  329 => 147,  323 => 144,  320 => 143,  318 => 142,  314 => 140,  301 => 137,  298 => 136,  294 => 135,  290 => 134,  277 => 123,  269 => 118,  260 => 112,  255 => 109,  249 => 105,  243 => 102,  240 => 101,  238 => 100,  234 => 98,  221 => 95,  218 => 94,  214 => 93,  210 => 92,  203 => 87,  197 => 84,  194 => 83,  192 => 82,  188 => 80,  175 => 77,  172 => 76,  168 => 75,  164 => 74,  159 => 71,  157 => 70,  147 => 65,  140 => 63,  134 => 60,  130 => 59,  124 => 56,  120 => 55,  116 => 53,  112 => 52,  107 => 50,  89 => 35,  85 => 34,  78 => 30,  74 => 29,  67 => 25,  63 => 24,  56 => 20,  52 => 19,  45 => 15,  41 => 14,  33 => 9,  23 => 1,);
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
\t\t
\t\t{% for record in records %}
\t\t<div class=\"box row line-mid ng-scope\" style=\"margin-top: 20px; border: 1px solid #ebebeb;\">
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

\t\t\t\t\tconsole.log(data[i]['address']);

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


</script>", "D:\\MgcBrowser/themes/magnachain/pages/address.htm", "");
    }
}
