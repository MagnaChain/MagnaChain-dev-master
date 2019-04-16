<?php

/* D:\MgcBrowser/themes/magnachain/partials/header-date.htm */
class __TwigTemplate_8051c6fea5e6421c8dbb98ffb8a7b5aac070e858982e3f16818bd1b9757b0fb7 extends Twig_Template
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
        echo "<nav class=\"nav navbar-default navbar-fixed-top\" style=\"background-color: #fff; height: 80px; width: 100%; border-bottom: 2px solid; border-bottom-color: #ebebeb; \">
    <div class=\"container nav-container\">
        <div class=\"navbar-header\">
            <!--在移动端的时候导航条折叠起来，三横的样式出现，点击该样式可以显示或隐藏导航条上的内容-->
            <button class=\"navbar-toggle\" data-toggle=\"collapse\" data-target=\"#menu\">
                <span class=\"icon-bar\"></span>
                <span class=\"icon-bar\"></span>
                <span class=\"icon-bar\"></span>
            </button>
            <a href=\"/\" class=\"navbar-brand\">Magna</a>
        </div>
        <div id=\"menu\" class=\"collapse navbar-collapse\" style=\"background-color: #fff;\">

            <ul class=\"nav navbar-nav\">
                <!-- <li class=\"Blocks\"><a href=\"#\">区块</a></li> -->
                <li class=\"Magna\"><a href=\"/all-blocks\">";
        // line 16
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["区块"]);
        echo "</a></li>
                <li class=\"Magna\"><a href=\"http://www.matgz.com/index.html\">MagnaChain</a></li>
            </ul>

            <div>
                
                <form class=\"navbar-form navbar-right hidden-xs hidden-sm\" role=\"search\" action=\"/search\" method=\"get\">
                    <div class=\"form-group hidden-xs hidden-sm\">
                        <a href=\"#\" data-request=\"onSwitchLocale\" data-request-data=\"locale: 'cn'\" style=\"color: #777;\">中文</a> |
                        <a href=\"#\" data-request=\"onSwitchLocale\" data-request-data=\"locale: 'en'\" style=\"color: #777; margin-right: 20px;\">English</a>
                        <input type=\"search\" name=\"search\" class=\"form-control\" placeholder=\"";
        // line 26
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["高度/块哈希/交易/地址"]);
        echo "\" style=\"width: 289px;\">
                    </div>
                </form>
                <form class=\"navbar-form navbar-right\" role=\"search\" action=\"/search\" method=\"get\">
                    <div class=\"form-group hidden-md hidden-lg\">
                        <input type=\"search\" name=\"search-sm\" class=\"form-control\" placeholder=\"";
        // line 31
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["高度/块哈希/交易/地址"]);
        echo "\"\">
                    </div>
                </form>
                <div class=\"input-group input-daterange hidden-sm hidden-md hidden-lg\" style=\"margin-bottom: 8px;\">
                    <input type=\"text\" class=\"btn btn-default\" id=\"datepicker\" value=\"选择日期\">
                </div>

            </div>

        </div>
    </div>
</nav>";
    }

    public function getTemplateName()
    {
        return "D:\\MgcBrowser/themes/magnachain/partials/header-date.htm";
    }

    public function isTraitable()
    {
        return false;
    }

    public function getDebugInfo()
    {
        return array (  61 => 31,  53 => 26,  40 => 16,  23 => 1,);
    }

    public function getSourceContext()
    {
        return new Twig_Source("<nav class=\"nav navbar-default navbar-fixed-top\" style=\"background-color: #fff; height: 80px; width: 100%; border-bottom: 2px solid; border-bottom-color: #ebebeb; \">
    <div class=\"container nav-container\">
        <div class=\"navbar-header\">
            <!--在移动端的时候导航条折叠起来，三横的样式出现，点击该样式可以显示或隐藏导航条上的内容-->
            <button class=\"navbar-toggle\" data-toggle=\"collapse\" data-target=\"#menu\">
                <span class=\"icon-bar\"></span>
                <span class=\"icon-bar\"></span>
                <span class=\"icon-bar\"></span>
            </button>
            <a href=\"/\" class=\"navbar-brand\">Magna</a>
        </div>
        <div id=\"menu\" class=\"collapse navbar-collapse\" style=\"background-color: #fff;\">

            <ul class=\"nav navbar-nav\">
                <!-- <li class=\"Blocks\"><a href=\"#\">区块</a></li> -->
                <li class=\"Magna\"><a href=\"/all-blocks\">{{ '区块'|_ }}</a></li>
                <li class=\"Magna\"><a href=\"http://www.matgz.com/index.html\">MagnaChain</a></li>
            </ul>

            <div>
                
                <form class=\"navbar-form navbar-right hidden-xs hidden-sm\" role=\"search\" action=\"/search\" method=\"get\">
                    <div class=\"form-group hidden-xs hidden-sm\">
                        <a href=\"#\" data-request=\"onSwitchLocale\" data-request-data=\"locale: 'cn'\" style=\"color: #777;\">中文</a> |
                        <a href=\"#\" data-request=\"onSwitchLocale\" data-request-data=\"locale: 'en'\" style=\"color: #777; margin-right: 20px;\">English</a>
                        <input type=\"search\" name=\"search\" class=\"form-control\" placeholder=\"{{ '高度/块哈希/交易/地址'|_ }}\" style=\"width: 289px;\">
                    </div>
                </form>
                <form class=\"navbar-form navbar-right\" role=\"search\" action=\"/search\" method=\"get\">
                    <div class=\"form-group hidden-md hidden-lg\">
                        <input type=\"search\" name=\"search-sm\" class=\"form-control\" placeholder=\"{{ '高度/块哈希/交易/地址'|_ }}\"\">
                    </div>
                </form>
                <div class=\"input-group input-daterange hidden-sm hidden-md hidden-lg\" style=\"margin-bottom: 8px;\">
                    <input type=\"text\" class=\"btn btn-default\" id=\"datepicker\" value=\"选择日期\">
                </div>

            </div>

        </div>
    </div>
</nav>", "D:\\MgcBrowser/themes/magnachain/partials/header-date.htm", "");
    }
}
