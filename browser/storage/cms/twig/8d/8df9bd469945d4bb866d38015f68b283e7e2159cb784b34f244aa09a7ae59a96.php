<?php

/* D:\MgcBrowser/themes/magnachain/pages/404.htm */
class __TwigTemplate_320ce8011c5dd6d83e4b077afc901f9eaec7df0f5bcbbb0937b180ff2582db03 extends Twig_Template
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
        if (($context["error"] ?? null)) {
            // line 2
            echo "<h1 style=\"margin-top: 100px; margin-bottom: 437px;\">";
            echo call_user_func_array($this->env->getFilter('_')->getCallable(), [":error", ["error" => ($context["error"] ?? null)]]);
            echo "</h1>
";
        } else {
            // line 4
            echo "<h1 style=\"margin-top: 100px; margin-bottom: 437px;\">";
            echo call_user_func_array($this->env->getFilter('_')->getCallable(), ["网页君迷路了~"]);
            echo "</h1>
";
        }
        // line 6
        echo "
<script>
\t
\t

</script>";
    }

    public function getTemplateName()
    {
        return "D:\\MgcBrowser/themes/magnachain/pages/404.htm";
    }

    public function isTraitable()
    {
        return false;
    }

    public function getDebugInfo()
    {
        return array (  37 => 6,  31 => 4,  25 => 2,  23 => 1,);
    }

    public function getSourceContext()
    {
        return new Twig_Source("{% if error %}
<h1 style=\"margin-top: 100px; margin-bottom: 437px;\">{{ ':error'|_({ error: error }) }}</h1>
{% else %}
<h1 style=\"margin-top: 100px; margin-bottom: 437px;\">{{ '网页君迷路了~'|_ }}</h1>
{% endif %}

<script>
\t
\t

</script>", "D:\\MgcBrowser/themes/magnachain/pages/404.htm", "");
    }
}
