{% for record in records %}
		<div class="box row line-mid ng-scope" id="tx-records-{{record.txhash}}" style="margin-top: 20px; border: 1px solid #ebebeb;">
			
			<a class="hidden-xs hidden-sm" href="/tx/{{ record.txhash }}" style="float: left; margin-top: 14px; margin-bottom: 14px; margin-left: 40px;">
				<span class="ellipsis">{{ record.txhash }}</span>
			</a>
			
			<a class="hidden-md hidden-lg" href="/tx/{{ record.txhash }}" style="float: left; margin-top: 14px; margin-bottom: 14px; margin-left: 20px; overflow: hidden; text-overflow:ellipsis; white-space: nowrap; width: 200px;">
				<span class="ellipsis">{{ record.txhash }}</span>
			</a>
			
			<span class="hidden-xs hidden-sm" style="float: right; margin-top: 14px; margin-bottom: 14px; margin-right: 60px;">{{ '完成时间'|_ }} {{ record.time }}</span>
		
			<span class="hidden-md hidden-lg" style="float: right; margin-top: 14px; margin-bottom: 14px; margin-right: 20px;">{{ '完成时间'|_ }} {{ record.time }}</span>

			<div style="margin-top: 49px; height: 1px; background-color: #ebebeb;"></div>
		
			<div class="row">
				{% if record.in %}

				<div class="col-md-5 col-xs-12 hidden-xs hidden-sm" style="margin-top: 20px; margin-left: 40px; float: left;">

				<div id="input_div_{{ record.txhash }}">
					{% for txin in record.in %}

					<div class="panel panel-default" id="input" style="background-color: #ebebeb; padding-top: 12px; padding-left: 12px;"><a href="/address/{{ txin.address }}" style="float: left;">{{ txin.address }}</a><p>{{ txin.inNum }} MGC</p></div>

					{% endfor %}
				</div>

				{% if record.more_in %}

				<button class="btn btn-default btn-md" id="btn_input_{{ record.txhash }}" style="float: left;" status="1">显示更多</button>

				{% endif %}

				</div>
		
				<div class="col-md-5 col-xs-12 hidden-md hidden-lg" style="margin-top: 20px;">

					<div id="input_div_{{ record.txhash }}">
						{% for txin in record.in %}

						<div class="panel panel-default" id="input" style="background-color: #ebebeb; padding-top: 12px; padding-left: 12px;"><a href="/address/{{ txin.address }}" style="float: left;">{{ txin.address }}</a><p>{{ txin.inNum }} MGC</p></div>

						{% endfor %}
					</div> 

					{% if record.more_in %}

					<button class="btn btn-default btn-md" id="btn_input_{{ record.txhash }}" style="float: left;" data-status="1">显示更多</button>

					{% endif %}

				</div>

				{% else %}

				<div class="col-md-5 col-xs-12 hidden-xs hidden-sm" style="margin-top: 20px; margin-left: 40px;">

					<div class="panel panel-default" style=" background-color: #ebebeb; padding-top: 12px; padding-left: 12px;"><p>{{ '没有交易输入'|_ }}</p></div>

				</div>

				<div class="col-md-5 col-xs-12 hidden-md hidden-lg" style="margin-top: 20px;">

					<div class="panel panel-default" style=" background-color: #ebebeb; padding-top: 12px; padding-left: 12px;"><p>{{ '没有交易输入'|_ }}</p></div>

				</div>

				{% endif %}

				<div class="col-md-1 col-xs-12" style="text-align: center; margin-top: 10px;">

					<div class="hidden-xs hidden-sm"><span style="font-size: 34px; color: #ebebeb;">＞</span></div>

					<div class="hidden-md hidden-lg"><span style="font-size: 34px; color: #ebebeb;">∨</span></div>

				</div>

				<div class="col-md-5 col-xs-12"  style="margin-top: 20px;">

					<div id="output_div_{{ record.txhash }}">
						{% for out in record.out %}

						<div class="panel panel-default" style="background-color: #ebebeb; padding-top: 12px; padding-left: 12px;"><a href="/address/{{ out.address }}" style="float: left;">{{ out.address }}</a><p>{{ out.outNum }} MGC</p></div>

						{% endfor %}
					</div>

					{% if record.more_output %}

					<button class="btn btn-default btn-md" id="btn_output_{{ record.txhash }}" style="float: left;">显示更多</button>

					{% endif %}

				</div>
					
			</div>

			<div style="border-top: 1px solid #ebebeb; margin-top: 10px;">
				{% if record.reward %}
					<div>
						<button type="button" class="btn btn-default btn-sm" disabled="disabled" style="float: left; margin-left: 10px; margin-bottom: 10px; margin-top: 10px;">{{ '矿工费'|_ }}： {{ record.reward }} MGC</button>
					</div>
				{% endif %}
				<div>
					<button type="button" class="btn btn-default btn-sm" disabled="disabled" style="float: right; margin-right: 10px; margin-bottom: 10px; margin-top: 10px;">{{ record.allOut }}MGC</button>
				</div>
			</div>
		
		</div>
		{% endfor %}