<?php

/* D:\MgcBrowser/themes/magnachain/layouts/mgc-date.htm */
class __TwigTemplate_beca033610644449fe0f65321eb0ecd8c9d9e0415e23f9dc7fa2f0cb5478e89b extends Twig_Template
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
        echo "<!DOCTYPE html>
<html lang=\"en\">
<head>
\t
\t<meta charset=\"UTF-8\">
\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">
\t<link rel=\"stylesheet\" href=\"/themes/magnachain/assets/css/bootstrap.min.css\">
\t<link rel=\"stylesheet\" href=\"/themes/magnachain/assets/css/mgc.css\">
\t<link rel=\"stylesheet\" href=\"/themes/magnachain/assets/css/bootstrap-datepicker3.min.css\">
\t<script type=\"text/javascript\" src=\"/themes/magnachain/assets/js/jquery.js\"></script>
\t<script type=\"text/javascript\" src=\"/themes/magnachain/assets/js/bootstrap.min.js\"></script>
\t<script type=\"text/javascript\" src=\"/themes/magnachain/assets/js/date.js\"></script>
\t<script type=\"text/javascript\" src=\"/themes/magnachain/assets/js/bootstrap-datepicker.js\"></script>
\t
\t<title>";
        // line 15
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["MGC浏览器"]);
        echo "</title>
</head>
<body>

\t<div class=\"header\">";
        // line 19
        $context['__cms_partial_params'] = [];
        echo $this->env->getExtension('Cms\Twig\Extension')->partialFunction("header-date.htm"        , $context['__cms_partial_params']        , true        );
        unset($context['__cms_partial_params']);
        echo "</div>

\t<div class=\"container-fluid\" style=\"min-height: 575px; margin-top: 82px;\">

\t\t";
        // line 23
        echo $this->env->getExtension('Cms\Twig\Extension')->pageFunction();
        // line 24
        echo "
\t</div>

\t<div class=\"footer\" style=\"margin-top: 131px;\">";
        // line 27
        $context['__cms_partial_params'] = [];
        echo $this->env->getExtension('Cms\Twig\Extension')->partialFunction("footer.htm"        , $context['__cms_partial_params']        , true        );
        unset($context['__cms_partial_params']);
        echo "</div>
\t";
        // line 28
        $_minify = System\Classes\CombineAssets::instance()->useMinify;
        if ($_minify) {
            echo '<script src="'. Request::getBasePath()
                    .'/modules/system/assets/js/framework.combined-min.js"></script>'.PHP_EOL;
        }
        else {
            echo '<script src="'. Request::getBasePath()
                    .'/modules/system/assets/js/framework.js"></script>'.PHP_EOL;
            echo '<script src="'. Request::getBasePath()
                    .'/modules/system/assets/js/framework.extras.js"></script>'.PHP_EOL;
        }
        echo '<link rel="stylesheet" property="stylesheet" href="'. Request::getBasePath()
                    .'/modules/system/assets/css/framework.extras'.($_minify ? '-min' : '').'.css">'.PHP_EOL;
        unset($_minify);
        // line 29
        echo "
</body>
</html>";
    }

    public function getTemplateName()
    {
        return "D:\\MgcBrowser/themes/magnachain/layouts/mgc-date.htm";
    }

    public function isTraitable()
    {
        return false;
    }

    public function getDebugInfo()
    {
        return array (  83 => 29,  68 => 28,  62 => 27,  57 => 24,  55 => 23,  46 => 19,  39 => 15,  23 => 1,);
    }

    public function getSourceContext()
    {
        return new Twig_Source("<!DOCTYPE html>
<html lang=\"en\">
<head>
\t
\t<meta charset=\"UTF-8\">
\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">
\t<link rel=\"stylesheet\" href=\"/themes/magnachain/assets/css/bootstrap.min.css\">
\t<link rel=\"stylesheet\" href=\"/themes/magnachain/assets/css/mgc.css\">
\t<link rel=\"stylesheet\" href=\"/themes/magnachain/assets/css/bootstrap-datepicker3.min.css\">
\t<script type=\"text/javascript\" src=\"/themes/magnachain/assets/js/jquery.js\"></script>
\t<script type=\"text/javascript\" src=\"/themes/magnachain/assets/js/bootstrap.min.js\"></script>
\t<script type=\"text/javascript\" src=\"/themes/magnachain/assets/js/date.js\"></script>
\t<script type=\"text/javascript\" src=\"/themes/magnachain/assets/js/bootstrap-datepicker.js\"></script>
\t
\t<title>{{ 'MGC浏览器'|_ }}</title>
</head>
<body>

\t<div class=\"header\">{% partial \"header-date.htm\" %}</div>

\t<div class=\"container-fluid\" style=\"min-height: 575px; margin-top: 82px;\">

\t\t{% page %}

\t</div>

\t<div class=\"footer\" style=\"margin-top: 131px;\">{% partial \"footer.htm\" %}</div>
\t{% framework extras %}

</body>
</html>", "D:\\MgcBrowser/themes/magnachain/layouts/mgc-date.htm", "");
    }
}
