<?php

/* D:\MgcBrowser/themes/magnachain/pages/tx.htm */
class __TwigTemplate_51b72298b0deabba4f47d22a5fb0cd823e50c0017e3900a22d3241a2b480e31e extends Twig_Template
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
\t<div class=\"table-responsive\">

\t\t<table class=\"table\">
\t\t\t<span class=\"ng-scope\" style=\"font-size: 24px; float: left; margin-top: -20px; margin-bottom: 20px;\">";
        // line 6
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["交易详情"]);
        echo "</span>
\t\t\t<tbody class=\"tbody\">
\t\t\t
\t\t\t";
        // line 9
        $context['_parent'] = $context;
        $context['_seq'] = twig_ensure_traversable(($context["details"] ?? null));
        foreach ($context['_seq'] as $context["_key"] => $context["detail"]) {
            // line 10
            echo "
\t\t     <tr style=\"height: 51px;\">
\t\t        <td><span style=\"margin-top: 7px; float: left; font-weight: bold;\">";
            // line 12
            echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["交易哈希"]);
            echo "</span></td>
\t\t        <td><span style=\"margin-top: 7px; float: right;\">";
            // line 13
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["detail"], "txhash", []), "html", null, true);
            echo "</span></td>
\t\t     </tr>

\t\t\t <tr style=\"height: 51px;\">
\t\t        <td><span style=\"margin-top: 7px; float: left; font-weight: bold;\">";
            // line 17
            echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["大小"]);
            echo "</span></td>
\t\t        <td><span style=\"margin-top: 7px; float: right;\">";
            // line 18
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["detail"], "txsize", []), "html", null, true);
            echo " Bytes</span></td>
\t\t     </tr>

\t\t     <tr style=\"height: 51px;\">
\t\t        <td><span style=\"margin-top: 7px; float: left; font-weight: bold;\">";
            // line 22
            echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["接收时间"]);
            echo "</span></td>
\t\t        <td><span style=\"margin-top: 7px; float: right;\">";
            // line 23
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["detail"], "time", []), "html", null, true);
            echo "</span></td>
\t\t     </tr>

\t\t     <tr style=\"height: 51px;\">
\t\t        <td><span style=\"margin-top: 7px; float: left; font-weight: bold;\">";
            // line 27
            echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["开采时间"]);
            echo "</span></td>
\t\t        <td><span style=\"margin-top: 7px; float: right;\">";
            // line 28
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["detail"], "time", []), "html", null, true);
            echo "</span></td>
\t\t     </tr>

\t\t     <tr style=\"height: 51px;\">
\t\t        <td><span style=\"margin-top: 7px; float: left; font-weight: bold;\">";
            // line 32
            echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["所属块"]);
            echo "</span></td>
\t\t        <td>
\t\t        \t<a href=\"/block/";
            // line 34
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["detail"], "height", []), "html", null, true);
            echo "\" style=\"margin-top: 7px; float: right;\">
\t\t\t\t\t\t<span class=\"ellipsis ng-binding\">";
            // line 35
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["detail"], "height", []), "html", null, true);
            echo "</span>
\t\t\t\t\t</a>
\t\t\t\t</td>
\t\t     </tr>

\t\t\t";
            // line 40
            if ((($context["reward"] ?? null) == true)) {
                // line 41
                echo "\t\t     <tr style=\"height: 51px;\">
\t\t        <td><span style=\"margin-top: 7px; float: left; font-weight: bold;\">";
                // line 42
                echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["挖矿奖励交易"]);
                echo "</span></td>
\t\t        <td><span style=\"margin-top: 7px; float: right;\">";
                // line 43
                echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["是"]);
                echo "</span></td>
\t\t     </tr>
\t\t\t";
            }
            // line 46
            echo "
\t\t\t";
        }
        $_parent = $context['_parent'];
        unset($context['_seq'], $context['_iterated'], $context['_key'], $context['detail'], $context['_parent'], $context['loop']);
        $context = array_intersect_key($context, $_parent) + $_parent;
        // line 48
        echo "
\t\t  </tbody>

\t\t</table>

\t</div>

\t<h1 style=\"font-size: 24px; text-align: left; margin-bottom: 20px; margin-top: 40px;\">";
        // line 55
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["交易记录"]);
        echo "</h1>

\t<div class=\"tx-record\">

\t\t";
        // line 59
        $context['_parent'] = $context;
        $context['_seq'] = twig_ensure_traversable(($context["records"] ?? null));
        foreach ($context['_seq'] as $context["_key"] => $context["record"]) {
            // line 60
            echo "\t\t<div class=\"box row line-mid ng-scope\" style=\"margin-top: 20px; border: 1px solid #ebebeb;\">
\t\t\t
\t\t\t<a class=\"hidden-xs hidden-sm\" href=\"javascript:void(0)\" style=\"float: left; margin-top: 14px; margin-bottom: 14px; margin-left: 40px;\">
\t\t\t\t<span class=\"ellipsis\">";
            // line 63
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "txhash", []), "html", null, true);
            echo "</span>
\t\t\t</a>
\t\t
\t\t\t<a class=\"hidden-md hidden-lg\" href=\"#\" style=\"float: left; margin-top: 14px; margin-bottom: 14px; margin-left: 20px; overflow: hidden; text-overflow:ellipsis; white-space: nowrap; width: 200px;\">
\t\t\t\t<span class=\"ellipsis\">";
            // line 67
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "txhash", []), "html", null, true);
            echo "</span>
\t\t\t</a>
\t\t\t
\t\t\t<span class=\"hidden-xs hidden-sm\" style=\"float: right; margin-top: 14px; margin-bottom: 14px; margin-right: 60px;\">";
            // line 70
            echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["完成时间"]);
            echo " ";
            echo twig_escape_filter($this->env, ($context["time"] ?? null), "html", null, true);
            echo "</span>
\t\t
\t\t\t<span class=\"hidden-md hidden-lg\" style=\"float: right; margin-top: 14px; margin-bottom: 14px; margin-right: 20px;\">";
            // line 72
            echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["完成时间"]);
            echo " ";
            echo twig_escape_filter($this->env, ($context["time"] ?? null), "html", null, true);
            echo "</span>

\t\t\t<div style=\"margin-top: 49px; height: 1px; background-color: #ebebeb;\"></div>
\t\t
\t\t\t<div class=\"row\">
\t\t\t\t";
            // line 77
            if (twig_get_attribute($this->env, $this->source, $context["record"], "in", [])) {
                // line 78
                echo "
\t\t\t\t<div class=\"col-md-5 col-xs-12 hidden-xs hidden-sm\" style=\"margin-top: 20px; margin-left: 40px; float: left;\">

\t\t\t\t<div id=\"input_div_";
                // line 81
                echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "txhash", []), "html", null, true);
                echo "\">
\t\t\t\t\t";
                // line 82
                $context['_parent'] = $context;
                $context['_seq'] = twig_ensure_traversable(twig_get_attribute($this->env, $this->source, $context["record"], "in", []));
                foreach ($context['_seq'] as $context["_key"] => $context["txin"]) {
                    // line 83
                    echo "
\t\t\t\t\t<div class=\"panel panel-default\" id=\"input\" style=\"background-color: #ebebeb; padding-top: 12px; padding-left: 12px;\"><a href=\"/address/";
                    // line 84
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
                // line 87
                echo "\t\t\t\t</div>

\t\t\t\t";
                // line 89
                if (twig_get_attribute($this->env, $this->source, $context["record"], "more_in", [])) {
                    // line 90
                    echo "
\t\t\t\t<button class=\"btn btn-default btn-md\" id=\"btn_input_";
                    // line 91
                    echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "txhash", []), "html", null, true);
                    echo "\" style=\"float: left;\" status=\"1\">";
                    echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["显示更多"]);
                    echo "</button>

\t\t\t\t";
                }
                // line 94
                echo "
\t\t\t\t</div>
\t\t
\t\t\t\t<div class=\"col-md-5 col-xs-12 hidden-md hidden-lg\" style=\"margin-top: 20px;\">

\t\t\t\t\t<div id=\"input_div_";
                // line 99
                echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "txhash", []), "html", null, true);
                echo "\">
\t\t\t\t\t\t";
                // line 100
                $context['_parent'] = $context;
                $context['_seq'] = twig_ensure_traversable(twig_get_attribute($this->env, $this->source, $context["record"], "in", []));
                foreach ($context['_seq'] as $context["_key"] => $context["txin"]) {
                    // line 101
                    echo "
\t\t\t\t\t\t<div class=\"panel panel-default\" id=\"input\" style=\"background-color: #ebebeb; padding-top: 12px; padding-left: 12px;\"><a href=\"/address/";
                    // line 102
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
                // line 105
                echo "\t\t\t\t\t</div> 

\t\t\t\t\t";
                // line 107
                if (twig_get_attribute($this->env, $this->source, $context["record"], "more_in", [])) {
                    // line 108
                    echo "
\t\t\t\t\t<button class=\"btn btn-default btn-md\" id=\"btn_input_";
                    // line 109
                    echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "txhash", []), "html", null, true);
                    echo "\" style=\"float: left;\" data-status=\"1\">";
                    echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["显示更多"]);
                    echo "</button>

\t\t\t\t\t";
                }
                // line 112
                echo "
\t\t\t\t</div>

\t\t\t\t";
            } else {
                // line 116
                echo "
\t\t\t\t<div class=\"col-md-5 col-xs-12 hidden-xs hidden-sm\" style=\"margin-top: 20px; margin-left: 40px;\">

\t\t\t\t\t<div class=\"panel panel-default\" style=\" background-color: #ebebeb; padding-top: 12px; padding-left: 12px;\"><p>";
                // line 119
                echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["没有交易输入"]);
                echo "</p></div>

\t\t\t\t</div>

\t\t\t\t<div class=\"col-md-5 col-xs-12 hidden-md hidden-lg\" style=\"margin-top: 20px;\">

\t\t\t\t\t<div class=\"panel panel-default\" style=\" background-color: #ebebeb; padding-top: 12px; padding-left: 12px;\"><p>";
                // line 125
                echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["没有交易输入"]);
                echo "</p></div>

\t\t\t\t</div>

\t\t\t\t";
            }
            // line 130
            echo "
\t\t\t\t<div class=\"col-md-1 col-xs-12\" style=\"text-align: center; margin-top: 10px;\">

\t\t\t\t\t<div class=\"hidden-xs hidden-sm\"><span style=\"font-size: 34px; color: #ebebeb;\">＞</span></div>

\t\t\t\t\t<div class=\"hidden-md hidden-lg\"><span style=\"font-size: 34px; color: #ebebeb;\">∨</span></div>

\t\t\t\t</div>
\t\t\t\t
\t\t\t\t<div class=\"col-md-5 col-xs-12\" style=\"margin-top: 20px;\">

\t\t\t\t\t<div id=\"output_div_";
            // line 141
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "txhash", []), "html", null, true);
            echo "\">
\t\t\t\t\t\t";
            // line 142
            $context['_parent'] = $context;
            $context['_seq'] = twig_ensure_traversable(twig_get_attribute($this->env, $this->source, $context["record"], "out", []));
            foreach ($context['_seq'] as $context["_key"] => $context["out"]) {
                // line 143
                echo "
\t\t\t\t\t\t<div class=\"panel panel-default\" style=\"background-color: #ebebeb; padding-top: 12px; padding-left: 12px;\"><a href=\"/address/";
                // line 144
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
            // line 147
            echo "\t\t\t\t\t</div>

\t\t\t\t\t";
            // line 149
            if (twig_get_attribute($this->env, $this->source, $context["record"], "more_output", [])) {
                // line 150
                echo "
\t\t\t\t\t<button class=\"btn btn-default btn-md\" id=\"btn_output_";
                // line 151
                echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "txhash", []), "html", null, true);
                echo "\" style=\"float: left;\">";
                echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["显示更多"]);
                echo "</button>

\t\t\t\t\t";
            }
            // line 154
            echo "
\t\t\t\t</div>
\t\t\t\t\t
\t\t\t</div>
\t\t
\t\t\t<div style=\"border-top: 1px solid #ebebeb; margin-top: 10px;\">
\t\t\t\t";
            // line 160
            if (($context["free"] ?? null)) {
                // line 161
                echo "\t\t\t\t\t<div>
\t\t\t\t\t\t<button type=\"button\" class=\"btn btn-default btn-sm\" disabled=\"disabled\" style=\"float: left; margin-left: 10px; margin-bottom: 10px; margin-top: 10px;\">";
                // line 162
                echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["矿工费"]);
                echo "： ";
                echo twig_escape_filter($this->env, ($context["free"] ?? null), "html", null, true);
                echo " MGC</button>
\t\t\t\t\t</div>
\t\t\t\t";
            }
            // line 165
            echo "\t\t\t\t<div>
\t\t\t\t\t<button type=\"button\" class=\"btn btn-default btn-sm\" disabled=\"disabled\" style=\"float: right; margin-right: 10px; margin-bottom: 10px; margin-top: 10px;\">";
            // line 166
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "allOut", []), "html", null, true);
            echo " MGC</button>
\t\t\t\t</div>
\t\t\t</div>
\t\t
\t\t</div>
\t\t";
        }
        $_parent = $context['_parent'];
        unset($context['_seq'], $context['_iterated'], $context['_key'], $context['record'], $context['_parent'], $context['loop']);
        $context = array_intersect_key($context, $_parent) + $_parent;
        // line 172
        echo "
\t</div>

</div>

<script>
\t
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
        return "D:\\MgcBrowser/themes/magnachain/pages/tx.htm";
    }

    public function isTraitable()
    {
        return false;
    }

    public function getDebugInfo()
    {
        return array (  396 => 172,  384 => 166,  381 => 165,  373 => 162,  370 => 161,  368 => 160,  360 => 154,  352 => 151,  349 => 150,  347 => 149,  343 => 147,  330 => 144,  327 => 143,  323 => 142,  319 => 141,  306 => 130,  298 => 125,  289 => 119,  284 => 116,  278 => 112,  270 => 109,  267 => 108,  265 => 107,  261 => 105,  248 => 102,  245 => 101,  241 => 100,  237 => 99,  230 => 94,  222 => 91,  219 => 90,  217 => 89,  213 => 87,  200 => 84,  197 => 83,  193 => 82,  189 => 81,  184 => 78,  182 => 77,  172 => 72,  165 => 70,  159 => 67,  152 => 63,  147 => 60,  143 => 59,  136 => 55,  127 => 48,  120 => 46,  114 => 43,  110 => 42,  107 => 41,  105 => 40,  97 => 35,  93 => 34,  88 => 32,  81 => 28,  77 => 27,  70 => 23,  66 => 22,  59 => 18,  55 => 17,  48 => 13,  44 => 12,  40 => 10,  36 => 9,  30 => 6,  23 => 1,);
    }

    public function getSourceContext()
    {
        return new Twig_Source("<div class=\"container content\">
\t
\t<div class=\"table-responsive\">

\t\t<table class=\"table\">
\t\t\t<span class=\"ng-scope\" style=\"font-size: 24px; float: left; margin-top: -20px; margin-bottom: 20px;\">{{ '交易详情'|_ }}</span>
\t\t\t<tbody class=\"tbody\">
\t\t\t
\t\t\t{% for detail in details %}

\t\t     <tr style=\"height: 51px;\">
\t\t        <td><span style=\"margin-top: 7px; float: left; font-weight: bold;\">{{ '交易哈希'|_ }}</span></td>
\t\t        <td><span style=\"margin-top: 7px; float: right;\">{{ detail.txhash }}</span></td>
\t\t     </tr>

\t\t\t <tr style=\"height: 51px;\">
\t\t        <td><span style=\"margin-top: 7px; float: left; font-weight: bold;\">{{ '大小'|_ }}</span></td>
\t\t        <td><span style=\"margin-top: 7px; float: right;\">{{ detail.txsize }} Bytes</span></td>
\t\t     </tr>

\t\t     <tr style=\"height: 51px;\">
\t\t        <td><span style=\"margin-top: 7px; float: left; font-weight: bold;\">{{ '接收时间'|_ }}</span></td>
\t\t        <td><span style=\"margin-top: 7px; float: right;\">{{ detail.time }}</span></td>
\t\t     </tr>

\t\t     <tr style=\"height: 51px;\">
\t\t        <td><span style=\"margin-top: 7px; float: left; font-weight: bold;\">{{ '开采时间'|_ }}</span></td>
\t\t        <td><span style=\"margin-top: 7px; float: right;\">{{ detail.time }}</span></td>
\t\t     </tr>

\t\t     <tr style=\"height: 51px;\">
\t\t        <td><span style=\"margin-top: 7px; float: left; font-weight: bold;\">{{ '所属块'|_ }}</span></td>
\t\t        <td>
\t\t        \t<a href=\"/block/{{ detail.height }}\" style=\"margin-top: 7px; float: right;\">
\t\t\t\t\t\t<span class=\"ellipsis ng-binding\">{{ detail.height }}</span>
\t\t\t\t\t</a>
\t\t\t\t</td>
\t\t     </tr>

\t\t\t{% if reward == true %}
\t\t     <tr style=\"height: 51px;\">
\t\t        <td><span style=\"margin-top: 7px; float: left; font-weight: bold;\">{{ '挖矿奖励交易'|_ }}</span></td>
\t\t        <td><span style=\"margin-top: 7px; float: right;\">{{ '是'|_ }}</span></td>
\t\t     </tr>
\t\t\t{% endif %}

\t\t\t{% endfor %}

\t\t  </tbody>

\t\t</table>

\t</div>

\t<h1 style=\"font-size: 24px; text-align: left; margin-bottom: 20px; margin-top: 40px;\">{{ '交易记录'|_ }}</h1>

\t<div class=\"tx-record\">

\t\t{% for record in records %}
\t\t<div class=\"box row line-mid ng-scope\" style=\"margin-top: 20px; border: 1px solid #ebebeb;\">
\t\t\t
\t\t\t<a class=\"hidden-xs hidden-sm\" href=\"javascript:void(0)\" style=\"float: left; margin-top: 14px; margin-bottom: 14px; margin-left: 40px;\">
\t\t\t\t<span class=\"ellipsis\">{{ record.txhash }}</span>
\t\t\t</a>
\t\t
\t\t\t<a class=\"hidden-md hidden-lg\" href=\"#\" style=\"float: left; margin-top: 14px; margin-bottom: 14px; margin-left: 20px; overflow: hidden; text-overflow:ellipsis; white-space: nowrap; width: 200px;\">
\t\t\t\t<span class=\"ellipsis\">{{ record.txhash }}</span>
\t\t\t</a>
\t\t\t
\t\t\t<span class=\"hidden-xs hidden-sm\" style=\"float: right; margin-top: 14px; margin-bottom: 14px; margin-right: 60px;\">{{ '完成时间'|_ }} {{ time }}</span>
\t\t
\t\t\t<span class=\"hidden-md hidden-lg\" style=\"float: right; margin-top: 14px; margin-bottom: 14px; margin-right: 20px;\">{{ '完成时间'|_ }} {{ time }}</span>

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

\t\t\t\t<button class=\"btn btn-default btn-md\" id=\"btn_input_{{ record.txhash }}\" style=\"float: left;\" status=\"1\">{{ '显示更多'|_ }}</button>

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

\t\t\t\t\t<button class=\"btn btn-default btn-md\" id=\"btn_input_{{ record.txhash }}\" style=\"float: left;\" data-status=\"1\">{{ '显示更多'|_ }}</button>

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
\t\t\t\t
\t\t\t\t<div class=\"col-md-5 col-xs-12\" style=\"margin-top: 20px;\">

\t\t\t\t\t<div id=\"output_div_{{ record.txhash }}\">
\t\t\t\t\t\t{% for out in record.out %}

\t\t\t\t\t\t<div class=\"panel panel-default\" style=\"background-color: #ebebeb; padding-top: 12px; padding-left: 12px;\"><a href=\"/address/{{ out.address }}\" style=\"float: left;\">{{ out.address }}</a><p>{{ out.outNum }} MGC</p></div>

\t\t\t\t\t\t{% endfor %}
\t\t\t\t\t</div>

\t\t\t\t\t{% if record.more_output %}

\t\t\t\t\t<button class=\"btn btn-default btn-md\" id=\"btn_output_{{ record.txhash }}\" style=\"float: left;\">{{ '显示更多'|_ }}</button>

\t\t\t\t\t{% endif %}

\t\t\t\t</div>
\t\t\t\t\t
\t\t\t</div>
\t\t
\t\t\t<div style=\"border-top: 1px solid #ebebeb; margin-top: 10px;\">
\t\t\t\t{% if free %}
\t\t\t\t\t<div>
\t\t\t\t\t\t<button type=\"button\" class=\"btn btn-default btn-sm\" disabled=\"disabled\" style=\"float: left; margin-left: 10px; margin-bottom: 10px; margin-top: 10px;\">{{ '矿工费'|_ }}： {{ free }} MGC</button>
\t\t\t\t\t</div>
\t\t\t\t{% endif %}
\t\t\t\t<div>
\t\t\t\t\t<button type=\"button\" class=\"btn btn-default btn-sm\" disabled=\"disabled\" style=\"float: right; margin-right: 10px; margin-bottom: 10px; margin-top: 10px;\">{{ record.allOut }} MGC</button>
\t\t\t\t</div>
\t\t\t</div>
\t\t
\t\t</div>
\t\t{% endfor %}

\t</div>

</div>

<script>
\t
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


</script>", "D:\\MgcBrowser/themes/magnachain/pages/tx.htm", "");
    }
}
