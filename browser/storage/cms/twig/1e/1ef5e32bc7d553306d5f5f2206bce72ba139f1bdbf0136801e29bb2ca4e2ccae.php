<?php

/* D:\MgcBrowser/themes/magnachain/layouts/mgc.htm */
class __TwigTemplate_77b708a9bbc51d7cb423c509975f8fc55302e0b27332ba29fb1b6654a27f7055 extends Twig_Template
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
\t<meta charset=\"UTF-8\">
\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">
\t<link rel=\"stylesheet\" href=\"/themes/magnachain/assets/css/bootstrap.min.css\">
\t<link rel=\"stylesheet\" href=\"/themes/magnachain/assets/css/mgc.css\">
\t<script type=\"text/javascript\" src=\"/themes/magnachain/assets/js/jquery.js\"></script>
\t<script type=\"text/javascript\" src=\"/themes/magnachain/assets/js/bootstrap.min.js\"></script>
\t
\t<title>";
        // line 11
        echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["MGC浏览器"]);
        echo "</title>
</head>
<body>
\t
\t<div class=\"header\">";
        // line 15
        $context['__cms_partial_params'] = [];
        echo $this->env->getExtension('Cms\Twig\Extension')->partialFunction("header.htm"        , $context['__cms_partial_params']        , true        );
        unset($context['__cms_partial_params']);
        echo "</div>

\t<div class=\"container-fluid\" style=\"min-height: 575px; margin-top: 82px;\">

\t\t";
        // line 19
        echo $this->env->getExtension('Cms\Twig\Extension')->pageFunction();
        // line 20
        echo "
\t</div>

\t<div class=\"footer\" style=\"margin-top: 131px;\">";
        // line 23
        $context['__cms_partial_params'] = [];
        echo $this->env->getExtension('Cms\Twig\Extension')->partialFunction("footer.htm"        , $context['__cms_partial_params']        , true        );
        unset($context['__cms_partial_params']);
        echo "</div>
\t";
        // line 24
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
        // line 25
        echo "
</body>
</html>";
    }

    public function getTemplateName()
    {
        return "D:\\MgcBrowser/themes/magnachain/layouts/mgc.htm";
    }

    public function isTraitable()
    {
        return false;
    }

    public function getDebugInfo()
    {
        return array (  79 => 25,  64 => 24,  58 => 23,  53 => 20,  51 => 19,  42 => 15,  35 => 11,  23 => 1,);
    }

    public function getSourceContext()
    {
        return new Twig_Source("<!DOCTYPE html>
<html lang=\"en\">
<head>
\t<meta charset=\"UTF-8\">
\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">
\t<link rel=\"stylesheet\" href=\"/themes/magnachain/assets/css/bootstrap.min.css\">
\t<link rel=\"stylesheet\" href=\"/themes/magnachain/assets/css/mgc.css\">
\t<script type=\"text/javascript\" src=\"/themes/magnachain/assets/js/jquery.js\"></script>
\t<script type=\"text/javascript\" src=\"/themes/magnachain/assets/js/bootstrap.min.js\"></script>
\t
\t<title>{{ 'MGC浏览器'|_ }}</title>
</head>
<body>
\t
\t<div class=\"header\">{% partial \"header.htm\" %}</div>

\t<div class=\"container-fluid\" style=\"min-height: 575px; margin-top: 82px;\">

\t\t{% page %}

\t</div>

\t<div class=\"footer\" style=\"margin-top: 131px;\">{% partial \"footer.htm\" %}</div>
\t{% framework extras %}

</body>
</html>", "D:\\MgcBrowser/themes/magnachain/layouts/mgc.htm", "");
    }
}
