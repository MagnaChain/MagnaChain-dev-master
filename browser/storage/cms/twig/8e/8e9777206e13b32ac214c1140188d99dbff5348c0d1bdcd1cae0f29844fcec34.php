<?php

/* D:\MgcBrowser/themes/magnachain/pages/block.htm */
class __TwigTemplate_48cec3ef946a0ee5613d194b55cceb85eb77d93af618d5192e60d155c25005ba extends Twig_Template
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

\t<div class=\"row block-detail\">
\t\t
\t\t";
        // line 5
        $context['_parent'] = $context;
        $context['_seq'] = twig_ensure_traversable(($context["block"] ?? null));
        foreach ($context['_seq'] as $context["_key"] => $context["info"]) {
            // line 6
            echo "
\t\t<div class=\"col-md-6\">
\t\t\t
\t\t\t<table class=\"table\">

\t\t\t\t<span class=\"ng-scope\" style=\"font-size: 24px; float: left; margin-top: -20px; margin-bottom: 20px;\">";
            // line 11
            echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["区块"]);
            echo " #";
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["info"], "height", []), "html", null, true);
            echo "</span>

\t\t\t\t<tbody class=\"tbody-left\">

\t\t\t\t     <tr style=\"height: 50px;\">
\t\t\t\t        <td><span style=\"margin-top: 5px; float: left; font-weight: bold;\">";
            // line 16
            echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["块哈希"]);
            echo "</span></td>
\t\t\t\t        <td>
\t\t\t\t        \t<span class=\"hidden-sm hidden-xs\" style=\"margin-top: 5px; float: right; word-break:normal; width:auto; display:block; white-space:pre-wrap;word-wrap : break-word ;overflow: hidden; width: 380px; text-align: right;\">";
            // line 18
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["info"], "blockhash", []), "html", null, true);
            echo "</span>
\t\t\t\t        \t<span class=\"hidden-md hidden-lg\" style=\"margin-top: 5px; float: right; word-break:normal; width:auto; display:block; white-space:pre-wrap;word-wrap : break-word ;overflow: hidden; width: 220px; text-align: right;\">";
            // line 19
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["info"], "blockhash", []), "html", null, true);
            echo "</span>
\t\t\t\t        </td>
\t\t\t\t     </tr>

\t\t\t\t\t <tr style=\"height: 50px;\">
\t\t\t\t        <td><span style=\"margin-top: 5px; float: left; font-weight: bold;\">";
            // line 24
            echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["高度"]);
            echo "</span></td>
\t\t\t\t        <td><span style=\"margin-top: 5px; float: right;\">";
            // line 25
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["info"], "height", []), "html", null, true);
            echo "</span></td>
\t\t\t\t     </tr>

\t\t\t\t     <tr style=\"height: 50px;\">
\t\t\t\t        <td><span style=\"margin-top: 5px; float: left; font-weight: bold;\">";
            // line 29
            echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["块奖励"]);
            echo "</span></td>
\t\t\t\t        <td><span style=\"margin-top: 5px; float: right;\">";
            // line 30
            echo twig_escape_filter($this->env, ($context["blockreward"] ?? null), "html", null, true);
            echo " MGC</span></td>
\t\t\t\t     </tr>

\t\t\t\t     <tr style=\"height: 50px;\">
\t\t\t\t        <td><span style=\"margin-top: 5px; float: left; font-weight: bold;\">";
            // line 34
            echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["时间"]);
            echo "</span></td>
\t\t\t\t        <td><span style=\"margin-top: 5px; float: right;\">";
            // line 35
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["info"], "time", []), "html", null, true);
            echo "</span></td>
\t\t\t\t     </tr>

\t\t\t\t     <tr style=\"height: 50px;\">
\t\t\t\t        <td><span style=\"margin-top: 5px; float: left; font-weight: bold;\">";
            // line 39
            echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["梅克莱根"]);
            echo "</span></td>
\t\t\t\t        <td>
\t\t\t\t        \t<span class=\"hidden-sm hidden-xs\" style=\"margin-top: 5px; float: right; word-break:normal; width:auto; display:block; white-space:pre-wrap;word-wrap : break-word ;overflow: hidden; width: 380px; text-align: right;\">";
            // line 41
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["info"], "hashmerkleroot", []), "html", null, true);
            echo "</span>
\t\t\t\t        \t<span class=\"hidden-md hidden-lg\" style=\"margin-top: 5px; float: right; word-break:normal; width:auto; display:block; white-space:pre-wrap;word-wrap : break-word ;overflow: hidden; width: 220px; text-align: right;\">";
            // line 42
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["info"], "hashmerkleroot", []), "html", null, true);
            echo "</span>
\t\t\t\t        </td>
\t\t\t\t     </tr>

\t\t\t\t     <tr style=\"height: 50px;\">
\t\t\t\t        <td><span style=\"margin-top: 5px; float: left; font-weight: bold;\">Nonce</span></td>
\t\t\t\t        <td><span style=\"margin-top: 5px; float: right;\">";
            // line 48
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["info"], "nonce", []), "html", null, true);
            echo "</span></td>
\t\t\t\t     </tr>

\t\t\t\t  </tbody>

\t\t\t</table>

\t\t</div>

\t\t<div class=\"col-md-6\">
\t\t\t
\t\t\t<table class=\"table\">

\t\t\t\t<span class=\"ng-scope\" style=\"visibility:hidden; font-size: 24px; float: left; margin-top: -20px; margin-bottom: 20px;\">";
            // line 61
            echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["交易详情"]);
            echo "</span>
\t\t\t\t
\t\t\t\t<tbody class=\"tbody-right\">

\t\t\t     <tr style=\"height: 50px;\">
\t\t\t        <td><span style=\"margin-top: 5px; float: left; font-weight: bold;\">";
            // line 66
            echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["交易数量"]);
            echo "</span></td>
\t\t\t        <td><span style=\"margin-top: 5px; float: right;\">";
            // line 67
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["info"], "num", []), "html", null, true);
            echo "</span></td>
\t\t\t     </tr>

\t\t\t     <tr style=\"height: 50px;\">
\t\t\t        <td><span style=\"margin-top: 5px; float: left; font-weight: bold;\">";
            // line 71
            echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["块大小"]);
            echo "</span></td>
\t\t\t        <td><span style=\"margin-top: 5px; float: right;\">";
            // line 72
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["info"], "blocksize", []), "html", null, true);
            echo " Bytes</span></td>
\t\t\t     </tr>

\t\t\t\t <tr style=\"height: 50px;\">
\t\t\t        <td><span style=\"margin-top: 5px; float: left; font-weight: bold;\">";
            // line 76
            echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["难度"]);
            echo "</span></td>
\t\t\t        <td><span style=\"margin-top: 5px; float: right;\">";
            // line 77
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["info"], "difficulty", []), "html", null, true);
            echo "</span></td>
\t\t\t     </tr>

\t\t\t     <tr style=\"height: 50px;\">
\t\t\t        <td><span style=\"margin-top: 5px; float: left; font-weight: bold;\">";
            // line 81
            echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["Bits"]);
            echo "</span></td>
\t\t\t        <td><span style=\"margin-top: 5px; float: right;\">";
            // line 82
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["info"], "bits", []), "html", null, true);
            echo "</span></td>
\t\t\t     </tr>

\t\t\t     <tr style=\"height: 50px;\">
\t\t\t        <td><span style=\"margin-top: 5px;float: left; font-weight: bold;\">";
            // line 86
            echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["版本"]);
            echo "</span></td>
\t\t\t        <td><span style=\"margin-top: 5px;float: right;\">";
            // line 87
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["info"], "version", []), "html", null, true);
            echo "</span></td>
\t\t\t     </tr>

\t\t\t     <tr style=\"height: 50px;\">
\t\t\t        <td><span style=\"margin-top: 5px; float: left; font-weight: bold;\">";
            // line 91
            echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["上一个块"]);
            echo "</span></td>
\t\t\t        <td>
\t\t\t        \t";
            // line 93
            if ((twig_get_attribute($this->env, $this->source, $context["info"], "height", []) > 0)) {
                // line 94
                echo "\t\t\t        \t<a href=\"/block/";
                echo twig_escape_filter($this->env, (twig_get_attribute($this->env, $this->source, $context["info"], "height", []) - 1), "html", null, true);
                echo "\" style=\"float: right; margin-top: 5px;\">
\t\t\t\t\t\t\t<span class=\"ellipsis ng-binding\"> ";
                // line 95
                echo twig_escape_filter($this->env, (twig_get_attribute($this->env, $this->source, $context["info"], "height", []) - 1), "html", null, true);
                echo "  </span>
\t\t\t\t\t\t</a>
\t\t\t\t\t\t";
            } elseif ((twig_get_attribute($this->env, $this->source,             // line 97
$context["info"], "height", []) == 0)) {
                // line 98
                echo "\t\t\t\t\t\t<a href=\"javascript:void(0);\" style=\"pointer-events:none; float: right; margin-top: 5px; color: #333;\">
\t\t\t\t\t\t\t<span class=\"ellipsis ng-binding\"> ";
                // line 99
                echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["无"]);
                echo " </span>
\t\t\t\t\t\t</a>
\t\t\t\t\t\t";
            }
            // line 102
            echo "\t\t\t\t\t</td>
\t\t\t     </tr>

\t\t\t     <tr style=\"height: 50px;\">
\t\t\t        <td><span style=\"margin-top: 5px; float: left; font-weight: bold;\">";
            // line 106
            echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["下一个块"]);
            echo "</span></td>
\t\t\t        <td>
\t\t\t        \t<a href=\"/block/";
            // line 108
            echo twig_escape_filter($this->env, (twig_get_attribute($this->env, $this->source, $context["info"], "height", []) + 1), "html", null, true);
            echo "\" style=\"float: right; margin-top: 5px;\">
\t\t\t\t\t\t\t<span class=\"ellipsis ng-binding\">";
            // line 109
            if ((twig_get_attribute($this->env, $this->source, $context["info"], "height", []) > 0)) {
                echo " ";
                echo twig_escape_filter($this->env, (twig_get_attribute($this->env, $this->source, $context["info"], "height", []) + 1), "html", null, true);
                echo " ";
            } elseif ((twig_get_attribute($this->env, $this->source, $context["info"], "height", []) == 0)) {
                echo " 1 ";
            }
            echo "</span>
\t\t\t\t\t\t</a>
\t\t\t\t\t</td>
\t\t\t     </tr>

\t\t\t  </tbody>

\t\t\t</table>

\t\t</div>

\t\t";
        }
        $_parent = $context['_parent'];
        unset($context['_seq'], $context['_iterated'], $context['_key'], $context['info'], $context['_parent'], $context['loop']);
        $context = array_intersect_key($context, $_parent) + $_parent;
        // line 121
        echo "
\t</div>

\t<h1 style=\"font-size: 24px; text-align: left; margin-bottom: 20px; margin-top: 40px;\">";
        // line 124
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["交易记录"]);
        echo "</h1>

\t<div class=\"tx-record\" >

\t\t";
        // line 128
        $context['_parent'] = $context;
        $context['_seq'] = twig_ensure_traversable(($context["records"] ?? null));
        foreach ($context['_seq'] as $context["_key"] => $context["record"]) {
            // line 129
            echo "\t\t<div class=\"box row line-mid ng-scope\" style=\"margin-top: 20px; border: 1px solid #ebebeb;\">
\t\t\t
\t\t\t<a class=\"hidden-xs hidden-sm\" href=\"/tx/";
            // line 131
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "txhash", []), "html", null, true);
            echo "\" style=\"float: left; margin-top: 14px; margin-bottom: 14px; margin-left: 40px;\">
\t\t\t\t<span class=\"ellipsis\">";
            // line 132
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "txhash", []), "html", null, true);
            echo "</span>
\t\t\t</a>
\t\t
\t\t\t<a class=\"hidden-md hidden-lg\" href=\"/tx/";
            // line 135
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "txhash", []), "html", null, true);
            echo "\" style=\"float: left; margin-top: 14px; margin-bottom: 14px; margin-left: 20px; overflow: hidden; text-overflow:ellipsis; white-space: nowrap; width: 200px;\">
\t\t\t\t<span class=\"ellipsis\">";
            // line 136
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "txhash", []), "html", null, true);
            echo "</span>
\t\t\t</a>
\t\t\t
\t\t\t<span class=\"hidden-xs hidden-sm\" style=\"float: right; margin-top: 14px; margin-bottom: 14px; margin-right: 60px;\">";
            // line 139
            echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["完成时间"]);
            echo " ";
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "time", []), "html", null, true);
            echo "</span>
\t\t
\t\t\t<span class=\"hidden-md hidden-lg\" style=\"float: right; margin-top: 14px; margin-bottom: 14px; margin-right: 20px;\">";
            // line 141
            echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["完成时间"]);
            echo " ";
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "time", []), "html", null, true);
            echo "</span>

\t\t\t<div style=\"margin-top: 49px; height: 1px; background-color: #ebebeb;\"></div>
\t\t
\t\t\t<div class=\"row\">
\t\t\t\t";
            // line 146
            if (twig_get_attribute($this->env, $this->source, $context["record"], "in", [])) {
                // line 147
                echo "
\t\t\t\t<div class=\"col-md-5 col-xs-12 hidden-xs hidden-sm\" style=\"margin-top: 20px; margin-left: 40px; float: left;\">

\t\t\t\t<div id=\"input_div_";
                // line 150
                echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "txhash", []), "html", null, true);
                echo "\">
\t\t\t\t\t";
                // line 151
                $context['_parent'] = $context;
                $context['_seq'] = twig_ensure_traversable(twig_get_attribute($this->env, $this->source, $context["record"], "in", []));
                foreach ($context['_seq'] as $context["_key"] => $context["txin"]) {
                    // line 152
                    echo "
\t\t\t\t\t<div class=\"panel panel-default\" id=\"input\" style=\"background-color: #ebebeb; padding-top: 12px; padding-left: 12px;\"><a href=\"/address/";
                    // line 153
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
                // line 156
                echo "\t\t\t\t</div>

\t\t\t\t";
                // line 158
                if (twig_get_attribute($this->env, $this->source, $context["record"], "more_in", [])) {
                    // line 159
                    echo "
\t\t\t\t<button class=\"btn btn-default btn-md\" id=\"btn_input_";
                    // line 160
                    echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "txhash", []), "html", null, true);
                    echo "\" style=\"float: left;\" status=\"1\">显示更多</button>

\t\t\t\t";
                }
                // line 163
                echo "
\t\t\t\t</div>
\t\t
\t\t\t\t<div class=\"col-md-5 col-xs-12 hidden-md hidden-lg\" style=\"margin-top: 20px;\">

\t\t\t\t\t<div id=\"input_div_";
                // line 168
                echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "txhash", []), "html", null, true);
                echo "\">
\t\t\t\t\t\t";
                // line 169
                $context['_parent'] = $context;
                $context['_seq'] = twig_ensure_traversable(twig_get_attribute($this->env, $this->source, $context["record"], "in", []));
                foreach ($context['_seq'] as $context["_key"] => $context["txin"]) {
                    // line 170
                    echo "
\t\t\t\t\t\t<div class=\"panel panel-default\" id=\"input\" style=\"background-color: #ebebeb; padding-top: 12px; padding-left: 12px;\"><a href=\"/address/";
                    // line 171
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
                // line 174
                echo "\t\t\t\t\t</div> 

\t\t\t\t\t";
                // line 176
                if (twig_get_attribute($this->env, $this->source, $context["record"], "more_in", [])) {
                    // line 177
                    echo "
\t\t\t\t\t<button class=\"btn btn-default btn-md\" id=\"btn_input_";
                    // line 178
                    echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "txhash", []), "html", null, true);
                    echo "\" style=\"float: left;\" data-status=\"1\">显示更多</button>

\t\t\t\t\t";
                }
                // line 181
                echo "
\t\t\t\t</div>

\t\t\t\t";
            } else {
                // line 185
                echo "
\t\t\t\t<div class=\"col-md-5 col-xs-12 hidden-xs hidden-sm\" style=\"margin-top: 20px; margin-left: 40px;\">

\t\t\t\t\t<div class=\"panel panel-default\" style=\" background-color: #ebebeb; padding-top: 12px; padding-left: 12px;\"><p>";
                // line 188
                echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["没有交易输入"]);
                echo "</p></div>

\t\t\t\t</div>

\t\t\t\t<div class=\"col-md-5 col-xs-12 hidden-md hidden-lg\" style=\"margin-top: 20px;\">

\t\t\t\t\t<div class=\"panel panel-default\" style=\" background-color: #ebebeb; padding-top: 12px; padding-left: 12px;\"><p>";
                // line 194
                echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["没有交易输入"]);
                echo "</p></div>

\t\t\t\t</div>

\t\t\t\t";
            }
            // line 199
            echo "
\t\t\t\t<div class=\"col-md-1 col-xs-12\" style=\"text-align: center; margin-top: 10px;\">

\t\t\t\t\t<div class=\"hidden-xs hidden-sm\"><span style=\"font-size: 34px; color: #ebebeb;\">＞</span></div>

\t\t\t\t\t<div class=\"hidden-md hidden-lg\"><span style=\"font-size: 34px; color: #ebebeb;\">∨</span></div>

\t\t\t\t</div>

\t\t\t\t<div class=\"col-md-5 col-xs-12\" style=\"margin-top: 20px;\">

\t\t\t\t\t<div id=\"output_div_";
            // line 210
            echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "txhash", []), "html", null, true);
            echo "\">
\t\t\t\t\t\t";
            // line 211
            $context['_parent'] = $context;
            $context['_seq'] = twig_ensure_traversable(twig_get_attribute($this->env, $this->source, $context["record"], "out", []));
            foreach ($context['_seq'] as $context["_key"] => $context["out"]) {
                // line 212
                echo "
\t\t\t\t\t\t<div class=\"panel panel-default\" style=\"background-color: #ebebeb; padding-top: 12px; padding-left: 12px;\"><a href=\"/address/";
                // line 213
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
            // line 216
            echo "\t\t\t\t\t</div>

\t\t\t\t\t";
            // line 218
            if (twig_get_attribute($this->env, $this->source, $context["record"], "more_output", [])) {
                // line 219
                echo "
\t\t\t\t\t<button class=\"btn btn-default btn-md\" id=\"btn_output_";
                // line 220
                echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "txhash", []), "html", null, true);
                echo "\" style=\"float: left;\">显示更多</button>

\t\t\t\t\t";
            }
            // line 223
            echo "
\t\t\t\t</div>
\t\t\t\t\t
\t\t\t</div>
\t\t
\t\t\t<div style=\"border-top: 1px solid #ebebeb; margin-top: 10px;\">
\t\t\t\t";
            // line 229
            if (twig_get_attribute($this->env, $this->source, $context["record"], "reward", [])) {
                // line 230
                echo "\t\t\t\t\t<div>
\t\t\t\t\t\t<button type=\"button\" class=\"btn btn-default btn-sm\" disabled=\"disabled\" style=\"float: left; margin-left: 10px; margin-bottom: 10px; margin-top: 10px;\">";
                // line 231
                echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["矿工费"]);
                echo "： ";
                echo twig_escape_filter($this->env, twig_get_attribute($this->env, $this->source, $context["record"], "reward", []), "html", null, true);
                echo " MGC</button>
\t\t\t\t\t</div>
\t\t\t\t";
            }
            // line 234
            echo "\t\t\t\t<div>
\t\t\t\t\t<button type=\"button\" class=\"btn btn-default btn-sm\" disabled=\"disabled\" style=\"float: right; margin-right: 10px; margin-bottom: 10px; margin-top: 10px;\">";
            // line 235
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
        // line 241
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
        return "D:\\MgcBrowser/themes/magnachain/pages/block.htm";
    }

    public function isTraitable()
    {
        return false;
    }

    public function getDebugInfo()
    {
        return array (  529 => 241,  517 => 235,  514 => 234,  506 => 231,  503 => 230,  501 => 229,  493 => 223,  487 => 220,  484 => 219,  482 => 218,  478 => 216,  465 => 213,  462 => 212,  458 => 211,  454 => 210,  441 => 199,  433 => 194,  424 => 188,  419 => 185,  413 => 181,  407 => 178,  404 => 177,  402 => 176,  398 => 174,  385 => 171,  382 => 170,  378 => 169,  374 => 168,  367 => 163,  361 => 160,  358 => 159,  356 => 158,  352 => 156,  339 => 153,  336 => 152,  332 => 151,  328 => 150,  323 => 147,  321 => 146,  311 => 141,  304 => 139,  298 => 136,  294 => 135,  288 => 132,  284 => 131,  280 => 129,  276 => 128,  269 => 124,  264 => 121,  240 => 109,  236 => 108,  231 => 106,  225 => 102,  219 => 99,  216 => 98,  214 => 97,  209 => 95,  204 => 94,  202 => 93,  197 => 91,  190 => 87,  186 => 86,  179 => 82,  175 => 81,  168 => 77,  164 => 76,  157 => 72,  153 => 71,  146 => 67,  142 => 66,  134 => 61,  118 => 48,  109 => 42,  105 => 41,  100 => 39,  93 => 35,  89 => 34,  82 => 30,  78 => 29,  71 => 25,  67 => 24,  59 => 19,  55 => 18,  50 => 16,  40 => 11,  33 => 6,  29 => 5,  23 => 1,);
    }

    public function getSourceContext()
    {
        return new Twig_Source("<div class=\"container content\">

\t<div class=\"row block-detail\">
\t\t
\t\t{% for info in block %}

\t\t<div class=\"col-md-6\">
\t\t\t
\t\t\t<table class=\"table\">

\t\t\t\t<span class=\"ng-scope\" style=\"font-size: 24px; float: left; margin-top: -20px; margin-bottom: 20px;\">{{ '区块'|_ }} #{{ info.height }}</span>

\t\t\t\t<tbody class=\"tbody-left\">

\t\t\t\t     <tr style=\"height: 50px;\">
\t\t\t\t        <td><span style=\"margin-top: 5px; float: left; font-weight: bold;\">{{ '块哈希'|_ }}</span></td>
\t\t\t\t        <td>
\t\t\t\t        \t<span class=\"hidden-sm hidden-xs\" style=\"margin-top: 5px; float: right; word-break:normal; width:auto; display:block; white-space:pre-wrap;word-wrap : break-word ;overflow: hidden; width: 380px; text-align: right;\">{{ info.blockhash }}</span>
\t\t\t\t        \t<span class=\"hidden-md hidden-lg\" style=\"margin-top: 5px; float: right; word-break:normal; width:auto; display:block; white-space:pre-wrap;word-wrap : break-word ;overflow: hidden; width: 220px; text-align: right;\">{{ info.blockhash }}</span>
\t\t\t\t        </td>
\t\t\t\t     </tr>

\t\t\t\t\t <tr style=\"height: 50px;\">
\t\t\t\t        <td><span style=\"margin-top: 5px; float: left; font-weight: bold;\">{{ '高度'|_ }}</span></td>
\t\t\t\t        <td><span style=\"margin-top: 5px; float: right;\">{{ info.height }}</span></td>
\t\t\t\t     </tr>

\t\t\t\t     <tr style=\"height: 50px;\">
\t\t\t\t        <td><span style=\"margin-top: 5px; float: left; font-weight: bold;\">{{ '块奖励'|_ }}</span></td>
\t\t\t\t        <td><span style=\"margin-top: 5px; float: right;\">{{ blockreward }} MGC</span></td>
\t\t\t\t     </tr>

\t\t\t\t     <tr style=\"height: 50px;\">
\t\t\t\t        <td><span style=\"margin-top: 5px; float: left; font-weight: bold;\">{{ '时间'|_ }}</span></td>
\t\t\t\t        <td><span style=\"margin-top: 5px; float: right;\">{{ info.time }}</span></td>
\t\t\t\t     </tr>

\t\t\t\t     <tr style=\"height: 50px;\">
\t\t\t\t        <td><span style=\"margin-top: 5px; float: left; font-weight: bold;\">{{ '梅克莱根'|_ }}</span></td>
\t\t\t\t        <td>
\t\t\t\t        \t<span class=\"hidden-sm hidden-xs\" style=\"margin-top: 5px; float: right; word-break:normal; width:auto; display:block; white-space:pre-wrap;word-wrap : break-word ;overflow: hidden; width: 380px; text-align: right;\">{{ info.hashmerkleroot }}</span>
\t\t\t\t        \t<span class=\"hidden-md hidden-lg\" style=\"margin-top: 5px; float: right; word-break:normal; width:auto; display:block; white-space:pre-wrap;word-wrap : break-word ;overflow: hidden; width: 220px; text-align: right;\">{{ info.hashmerkleroot }}</span>
\t\t\t\t        </td>
\t\t\t\t     </tr>

\t\t\t\t     <tr style=\"height: 50px;\">
\t\t\t\t        <td><span style=\"margin-top: 5px; float: left; font-weight: bold;\">Nonce</span></td>
\t\t\t\t        <td><span style=\"margin-top: 5px; float: right;\">{{ info.nonce }}</span></td>
\t\t\t\t     </tr>

\t\t\t\t  </tbody>

\t\t\t</table>

\t\t</div>

\t\t<div class=\"col-md-6\">
\t\t\t
\t\t\t<table class=\"table\">

\t\t\t\t<span class=\"ng-scope\" style=\"visibility:hidden; font-size: 24px; float: left; margin-top: -20px; margin-bottom: 20px;\">{{ '交易详情'|_ }}</span>
\t\t\t\t
\t\t\t\t<tbody class=\"tbody-right\">

\t\t\t     <tr style=\"height: 50px;\">
\t\t\t        <td><span style=\"margin-top: 5px; float: left; font-weight: bold;\">{{ '交易数量'|_ }}</span></td>
\t\t\t        <td><span style=\"margin-top: 5px; float: right;\">{{ info.num }}</span></td>
\t\t\t     </tr>

\t\t\t     <tr style=\"height: 50px;\">
\t\t\t        <td><span style=\"margin-top: 5px; float: left; font-weight: bold;\">{{ '块大小'|_ }}</span></td>
\t\t\t        <td><span style=\"margin-top: 5px; float: right;\">{{ info.blocksize }} Bytes</span></td>
\t\t\t     </tr>

\t\t\t\t <tr style=\"height: 50px;\">
\t\t\t        <td><span style=\"margin-top: 5px; float: left; font-weight: bold;\">{{ '难度'|_ }}</span></td>
\t\t\t        <td><span style=\"margin-top: 5px; float: right;\">{{ info.difficulty }}</span></td>
\t\t\t     </tr>

\t\t\t     <tr style=\"height: 50px;\">
\t\t\t        <td><span style=\"margin-top: 5px; float: left; font-weight: bold;\">{{ 'Bits'|_ }}</span></td>
\t\t\t        <td><span style=\"margin-top: 5px; float: right;\">{{ info.bits }}</span></td>
\t\t\t     </tr>

\t\t\t     <tr style=\"height: 50px;\">
\t\t\t        <td><span style=\"margin-top: 5px;float: left; font-weight: bold;\">{{ '版本'|_ }}</span></td>
\t\t\t        <td><span style=\"margin-top: 5px;float: right;\">{{ info.version }}</span></td>
\t\t\t     </tr>

\t\t\t     <tr style=\"height: 50px;\">
\t\t\t        <td><span style=\"margin-top: 5px; float: left; font-weight: bold;\">{{ '上一个块'|_ }}</span></td>
\t\t\t        <td>
\t\t\t        \t{% if info.height > 0 %}
\t\t\t        \t<a href=\"/block/{{ info.height-1 }}\" style=\"float: right; margin-top: 5px;\">
\t\t\t\t\t\t\t<span class=\"ellipsis ng-binding\"> {{ info.height-1 }}  </span>
\t\t\t\t\t\t</a>
\t\t\t\t\t\t{% elseif info.height == 0 %}
\t\t\t\t\t\t<a href=\"javascript:void(0);\" style=\"pointer-events:none; float: right; margin-top: 5px; color: #333;\">
\t\t\t\t\t\t\t<span class=\"ellipsis ng-binding\"> {{ '无'|_ }} </span>
\t\t\t\t\t\t</a>
\t\t\t\t\t\t{% endif %}
\t\t\t\t\t</td>
\t\t\t     </tr>

\t\t\t     <tr style=\"height: 50px;\">
\t\t\t        <td><span style=\"margin-top: 5px; float: left; font-weight: bold;\">{{ '下一个块'|_ }}</span></td>
\t\t\t        <td>
\t\t\t        \t<a href=\"/block/{{ info.height+1 }}\" style=\"float: right; margin-top: 5px;\">
\t\t\t\t\t\t\t<span class=\"ellipsis ng-binding\">{% if info.height > 0 %} {{ info.height+1 }} {% elseif info.height == 0 %} 1 {% endif %}</span>
\t\t\t\t\t\t</a>
\t\t\t\t\t</td>
\t\t\t     </tr>

\t\t\t  </tbody>

\t\t\t</table>

\t\t</div>

\t\t{% endfor %}

\t</div>

\t<h1 style=\"font-size: 24px; text-align: left; margin-bottom: 20px; margin-top: 40px;\">{{ '交易记录'|_ }}</h1>

\t<div class=\"tx-record\" >

\t\t{% for record in records %}
\t\t<div class=\"box row line-mid ng-scope\" style=\"margin-top: 20px; border: 1px solid #ebebeb;\">
\t\t\t
\t\t\t<a class=\"hidden-xs hidden-sm\" href=\"/tx/{{ record.txhash }}\" style=\"float: left; margin-top: 14px; margin-bottom: 14px; margin-left: 40px;\">
\t\t\t\t<span class=\"ellipsis\">{{ record.txhash }}</span>
\t\t\t</a>
\t\t
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

\t\t\t\t<div class=\"col-md-5 col-xs-12\" style=\"margin-top: 20px;\">

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
\t\t
\t\t\t<div style=\"border-top: 1px solid #ebebeb; margin-top: 10px;\">
\t\t\t\t{% if record.reward %}
\t\t\t\t\t<div>
\t\t\t\t\t\t<button type=\"button\" class=\"btn btn-default btn-sm\" disabled=\"disabled\" style=\"float: left; margin-left: 10px; margin-bottom: 10px; margin-top: 10px;\">{{ '矿工费'|_ }}： {{ record.reward }} MGC</button>
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


</script>", "D:\\MgcBrowser/themes/magnachain/pages/block.htm", "");
    }
}
