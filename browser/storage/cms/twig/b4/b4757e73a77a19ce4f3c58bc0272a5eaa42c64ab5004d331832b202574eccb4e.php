<?php

/* D:\MgcBrowser/themes/magnachain/partials/footer.htm */
class __TwigTemplate_f606211ff477ba1ae774846e62fdfaebf6be9fc41fdab83a1c89385787ecfe47 extends Twig_Template
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
        echo "<div class=\"footer-container\">
\t<div class=\"footer-logo\"><h1>Magna</h1></div>
\t<div class=\"links\">
\t\t<div><a href=\"#\" style=\"color: #333333;text-decoration: none;\">magnachain.co</a><span>   copyright © 2018 MGC Foundation</span></div>
\t</div>
</div>";
    }

    public function getTemplateName()
    {
        return "D:\\MgcBrowser/themes/magnachain/partials/footer.htm";
    }

    public function getDebugInfo()
    {
        return array (  23 => 1,);
    }

    public function getSourceContext()
    {
        return new Twig_Source("<div class=\"footer-container\">
\t<div class=\"footer-logo\"><h1>Magna</h1></div>
\t<div class=\"links\">
\t\t<div><a href=\"#\" style=\"color: #333333;text-decoration: none;\">magnachain.co</a><span>   copyright © 2018 MGC Foundation</span></div>
\t</div>
</div>", "D:\\MgcBrowser/themes/magnachain/partials/footer.htm", "");
    }
}
